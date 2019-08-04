<template>
    <v-layout row>
        <v-flex>
            <v-card height="100vh">
                <v-card-title class="blue white--text elevation-3">
                    <span class="headline">
                        Ministo
                        <small><em><small>v{{version}}</small></em></small>
                    </span>

                    <v-spacer></v-spacer>

                    <img src="~@/assets/icons/png/48x48.png" alt="logo">
                </v-card-title>

                <v-card-text>
                    <h3>
                        Ready to start Token Mining? Let's GO!
                    </h3>

                    <p class="mt-3">
                        Open your web browser to:
                        <br /><strong>https://minado.network/#/tag/{{tag}}</strong>
                        <v-icon
                            small
                            class="ml-2 red--text"
                            @click="open('https://minado.network/#/tag/' + tag)"
                        >
                            fa-external-link-alt
                        </v-icon>
                    </p>

                    <v-alert
                        :value="true"
                        type="info"
                        color="light-blue"
                        class="elevation-1 mt-4"
                        outline
                    >
                        <h3>THIS IS YOUR MINISTO TAG</h3>
                    </v-alert>

                    <h1 class="text-xs-center mt-2 tag">{{tag}}</h1>

                    <v-alert
                        :value="true"
                        type="info"
                        color="light-blue"
                        class="elevation-1 mt-3"
                        outline
                    >
                        <h3>CURRENT MINISTO STATUS</h3>
                    </v-alert>

                    <div class="system-status mt-2">
                        <div class="left-side">
                            <ul>
                                <li>Windows 10</li>
                                <li>8GB Ram</li>
                                <li>FAST Connectivity</li>
                            </ul>
                        </div>

                        <div class="right-side">
                            <div>
                                Hash Rate: <span class="red--text">{{hashRate}}</span>
                            </div>

                            <div>
                                # Hashes: {{numHashes}}
                            </div>

                            <div>
                                Lodes Delivered: <span class="red--text">{{numLodesDisplay}}</span>
                            </div>
                        </div>
                    </div>

                    <div class="activity-graph elevation-1">
                        [ REAL-TIME ACTIVITY GRAPH ]
                    </div>
                </v-card-text>
            </v-card>
        </v-flex>
    </v-layout>
</template>

<script>
import {
    mapActions,
    mapGetters
} from 'vuex'

/* Import libraries. */
import numeral from 'numeral'
import Store from 'electron-store'
import SockJS from 'sockjs-client'

/* Require modules. */
const ipc = require('electron').ipcRenderer
const web3Utils = require('web3-utils')

/* Initialize new (persistent) store. */
const store = new Store()

/* Import package.json. */
const pjson = require('../../../package.json')

/* Initialize "hybrid" ministo. */
const HybridMinisto = require('../../../build/Release/hybrid_ministo')

/* Initialize Minado.Network endpoint. */
// const MINADO_NETWORK_URL = 'ws://asia.minado.network'
const MINADO_NETWORK_URL = 'http://asia.minado.network'
// const MINADO_NETWORK_URL = 'http://dev.minado.network'

/* Initailize constants. */
const PRINT_STATS_TIMEOUT = 5000

const UINT256_LENGTH = 32

let GPUMiner = null // eslint-disable-line no-unused-vars

export default {
    data: () => ({
        ws: null, // WebSocket

        isMining: false,
        minadoAddress: '',
        minadoChallenge: '',
        minadoDifficulty: '',
        minadoTarget: '',

        hashRate: 'n/a',
        numHashes: 0,
        numLodes: 0,

        avatarSize: 64,
        tag: '',
        showGaia: false,
        pingTimeout: null,

        version: '',
        menuItems: [
            { title: 'Compact View' },
            { title: 'Exit' }
        ]
    }),
    computed: {
        ...mapGetters([
            'account',
            'email'
        ]),
        challengeDisplay () {
            if (this.minadoChallenge) {
                return this.minadoChallenge
            } else {
                return 'n/a'
            }
        },
        emailAddress () {
            return this.email
            // return this.$store.state.Profile.email
        },
        numLodesDisplay () {
            return numeral(this.numLodes).format('0,0')
        }
    },
    methods: {
        ...mapActions([
            'clearEmail',
            'updateEmail'
        ]),

        /* Initialization. */
        async init () {
            /* Set version. */
            this.version = pjson.version

            /* Get/validate ministo tag. */
            this.getTag()

            this.ws = new SockJS(MINADO_NETWORK_URL)
            // console.log('this.ws', this.ws)

            /* Handle 'opened' connection. */
            this.ws.onopen = () => {
                console.info('Minado.Network connection opened successfully!')

                /* Build "authorization" package. */
                const pkg = {
                    action: 'authorize',
                    client: 'ministo',
                    tag: this.tag,
                    version: this.version
                }

                /* Send package. */
                this.ws.send(JSON.stringify(pkg))
            }

            this.ws.onmessage = (_e) => {
                console.log('Incoming message:', _e.data)

                /* Initialize data. */
                let data = null

                try {
                    /* Parse JSON data. */
                    data = JSON.parse(_e.data)
                } catch (_e) {
                    console.error(_e)
                }

                if (data && data.action) {
                    /* Retrieve action. */
                    const action = data.action

                    // console.log(`Received Action [ ${action} ]`)

                    /* Mining notification (from server). */
                    // NOTE: Broadcast to all miners.
                    if (action === 'notify') {
                        /* Stop the miner. */
                        HybridMinisto.stop()

                        /* Give it a sec. */
                        setTimeout(() => {
                            this.minadoAddress = data.address
                            this.minadoChallenge = data.challenge
                            this.minadoDifficulty = data.difficulty
                            this.minadoTarget = data.target

                            /* Start the miner. */
                            this.mine()
                        }, 1000)
                    }

                    /* Start mining. */
                    if (action === 'start_mining' && data.tag === this.tag) {
                        /* Validate running instance. */
                        if (!this.isMining) {
                            /* Start the miner. */
                            this.mine()
                        }
                    }

                    /* Stop mining. */
                    if (action === 'stop_mining' && data.tag === this.tag) {
                        /* Stop the miner. */
                        HybridMinisto.stop()
                    }
                }
            }

            this.ws.onclose = () => {
                console.log('Connection closed.')

                /* Try to reconnect after 10 seconds. */
                setTimeout(() => {
                    console.log('Trying to re-connect...')

                    /* Re-initialize. */
                    this.init()
                }, 10000)
            }

            /* Initialize CPU Miner listener. */
            ipc.on('startMining', (_event, _arg) => {
                this.mine()
            })

            /* Initialize change tag listener. */
            ipc.on('changeTag', (_event, _arg) => {
                this.changeTag()
            })

            /* Initialize change tag listener. */
            // FIXME: Why do we need to call this thru here??
            ipc.on('openDevTools', (_event, _arg) => {
                ipc.send('openDevTools')
            })

            /* Initialize hash update listener. */
            ipc.on('updateNumHashes', (_event, _arg) => {
                this.numHashes = _arg
                // ipc.send('_debug', `this.numHashes [ ${this.numHashes} ]`)
            })

            /* Initialize hash update listener. */
            ipc.on('updateHashRate', (_event, _arg) => {
                this.hashRate = _arg
                // ipc.send('_debug', `this.hashRate [ ${this.hashRate} ]`)
            })

            /* Set an interval to update mining stats. */
            setInterval(
                () => { this.printMiningStats() }, PRINT_STATS_TIMEOUT
            )

            /* Handle (process) exit. */
            process.on('exit', () => {
                console.log('Process exiting... stopping miner')

                /* Stop the CPU miner. */
                HybridMinisto.stop()
            })
        },

        /* Open web browser hyperlink. */
        open (link) {
            this.$electron.shell.openExternal(link)
        },

        /* Retrieve the current tag. */
        getTag () {
            /* Retrieve tag from storage. */
            let tag = store.get('settings.tag')

            /* Validate tag. */
            if (!tag || tag.length !== 10) {
                /* Generate ministo tag. */
                tag = this._makeTag(10)

                /* Save tag to store. */
                store.set('settings.tag', tag)
            }

            ipc.send('_debug', `Ministo Tag [ ${tag} ]`)

            /* Update instance. */
            this.tag = tag

            /* Return tag. */
            return tag
        },

        changeTag () {
            /* Generate ministo tag. */
            this.tag = this._makeTag(10)

            /* Save tag to store. */
            store.set('settings.tag', this.tag)

            ipc.send('_debug', `Ministo tag has been changed to [ ${this.tag} ]`)
        },

        /* Update "native" miner parameters. */
        updateHybridMinisto () {
            // ipc.send('_debug', `updateHybridMinisto - ${this.minadoAddress} | ${this.minadoChallenge} | ${this.minadoTarget}`)

            /* Set hardware type. */
            // NOTE: Allowed values are either `cpu` or `cuda`.
            // FIXME: Auto-detect CUDA GPU compatiblity.
            HybridMinisto.setHardwareType('cpu')

            /* Set minter's address. */
            HybridMinisto.setMinterAddress(this.minadoAddress)

            /* Validate challenge (number). */
            // NOTE: `ethers..toHexString()` is truncating `00` from prefix.
            if (this.minadoChallenge.length !== UINT256_LENGTH * 2 + 2) {
                throw new Error(`Challenge length is incorrect [ ${this.minadoChallenge} ]`)
            }

            /* Set challenge (number). */
            HybridMinisto.setChallenge(this.minadoChallenge)

            /* Validate (difficulty) target. */
            // NOTE: `ethers..toHexString()` is truncating `00` from prefix.
            // if (this.minadoTarget.length !== UINT256_LENGTH * 2 + 2) {
            //     throw new Error(`Target length is incorrect [ ${this.minadoTarget} ]`)
            // }

            /* Set (difficulty) target. */
            HybridMinisto.setTarget(this.minadoTarget)
        },

        /* Hybrid miner. */
        mine () {
            ipc.send('_debug', `Ministo has started mining..`)

            // ipc.send('_debug', `TARGET [ ${this.minadoTarget} ]`)
            // ipc.send('_debug', `DIFFICULTY [ ${this.minadoDifficulty} ]`)

            const _verifyAndSubmit = (_solution) => {
                // ipc.send('_debug', `SOLUTION NUMBER [ ${_solution} ]`)

                /* Build digest hash. */
                const digest = web3Utils.soliditySha3(
                    this.minadoChallenge,
                    this.minadoAddress,
                    _solution
                )

                /* Calculate (big number) digest. */
                const digestBN = web3Utils.toBN(digest)

                /* Calculate (big number) target. */
                const targetBN = web3Utils.toBN(this.minadoTarget)

                if (digestBN.lte(targetBN)) {
                    ipc.send('_debug', `Submit mined solution [ ${_solution} ] for challenge [ ${this.minadoChallenge} ] with digest [ ${digest} ]`)

                    /* Build package. */
                    const pkg = {
                        action: 'submit',
                        // client: 'ministo',
                        // tag: this.tag,
                        // version: this.version,
                        token: '0xf6E9Fc9eB4C20eaE63Cb2d7675F4dD48B008C531', // ZeroGold
                        solution: _solution,
                        digest,
                        challenge: this.minadoChallenge,
                        difficulty: this.minadoDifficulty
                    }

                    /* Send package. */
                    this.ws.send(JSON.stringify(pkg))

                    /* Increment number of lodes. */
                    // FIXME: Add support for larger lodes.
                    this.numLodes++
                } else {
                    ipc.send('_debug', `
    Verification Failed!
    --------------------

        Challenge : ${this.minadoChallenge}
        Address   : ${this.minadoAddress}
        Solution  : ${_solution}
        Digest    : ${digestBN}
        Target    : ${this.minadoTarget}
                    `)
                }
            } // verifyAndSubmit

            HybridMinisto.stop()

            /* Update the CPU miner's parameters. */
            this.updateHybridMinisto()

            /* Set flag. */
            this.isMining = true

            HybridMinisto.run((err, sol) => {
                if (err) {
                    ipc.send('_debug', `ERROR: Failed to run 'HybridMinisto'. [ ${err} ]`)
                }

                if (sol) {
                    _verifyAndSubmit(sol)
                }

                /* Set flag. */
                this.isMining = false
            })
        },

        /***********************************************************************
         * Utilities
         */

        /* Create a new tag. */
        _makeTag (_length) {
            /* Initialize tag. */
            let tag = ''

            /* Initialize ALL "possible" characters. */
            const possible = '23456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz' // base56

            /* Loop through, `_length` times, generating random characters. */
            for (let i = 0; i < _length; i++) {
                tag += possible.charAt(Math.floor(Math.random() * possible.length))
            }

            /* Return tag. */
            return tag
        },

        /* Print/update mining stats. */
        printMiningStats () {
            /* Set hashes. */
            const hashes = HybridMinisto.hashes()

            /* Calucate hashrate. */
            const rate = hashes / PRINT_STATS_TIMEOUT / 1000

            /* Send update for number of hashes. */
            ipc.send('numHashes', numeral(hashes).format('0,0'))

            /* Send update for (formatted) hash rate. */
            ipc.send('hashRate', `${numeral(rate).format('0,0.00')} MH/s`)
        }
    },
    mounted: async function () {
        /* Initialize. */
        this.init()
    }
}
</script>

<style lang="scss">
.system-status {
    display: flex;
    justify-content: space-between;
}

.left-side {
    display: flex;
    flex-direction: column;
}

.right-side {
    width: 45vw;
    display: flex;
    flex-direction: column;
}

.activity-graph {
    width: 100%;
    height: 50px;
    margin-top: 10px;
    padding: 12px;

    color: white;
    font-size: 1.2em;
    font-weight: bold;

    text-align: center;

    background-color: rgba(45, 120, 220, 0.5);
    border: 1pt solid #333;
}

.tag {
    letter-spacing: 5px;
}
</style>
