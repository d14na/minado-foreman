<template>
    <div>
        <span class="headline">
            Minado Foreman
            <small><em><small>v{{version}}</small></em></small>
        </span>

        <img src="~@/assets/icons/png/48x48.png" alt="logo">

        <h3>
            Ready to start Token Mining? Let's GO!
        </h3>

        <p class="mt-3">
            Open your web browser to:
            <br />
            <strong @click="open('https://minado.network/#/tag/' + tag)">
                https://minado.network/#/tag/{{tag}}</strong>
        </p>

        <h3>THIS IS YOUR FOREMAN TAG</h3>

        <h1 class="text-xs-center mt-2 tag">{{tag}}</h1>

        <h3>CURRENT FOREMAN STATUS</h3>

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
                    Hash Rate: <span class="red--text">{{hashRateDisplay}}</span>
                </div>

                <div>
                    # Hashes: {{totalHashesDisplay}}
                </div>

                <div>
                    Lodes Delivered: <span class="red--text">{{numLodesDisplay}}</span>
                </div>
            </div>
        </div>

        <div class="activity-graph elevation-1">
            {{realtime}}
        </div>
    </div>
</template>

<script>
/* Import libraries. */
import moment from 'moment'
import numeral from 'numeral'
import Store from 'electron-store'
import SockJS from 'sockjs-client'

/* Require modules. */
const ipc = require('electron').ipcRenderer
const web3Utils = require('web3-utils')

/* Initialize new (persistent) store. */
const store = new Store()

/* Import package.json. */
const pjson = require('../../package.json')

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
        ws: null, // websocket
        ps: null, // system process

        isMining: false,
        minadoAddress: '',
        minadoChallenge: '',
        minadoDifficulty: '',
        minadoTarget: '',

        realtime: 'REAL-TIME ACTIVITY GRAPH',

        hashStartTime: 0,
        hashRate: web3Utils.toBN(0), // this is a BigNumber
        lastHashes: web3Utils.toBN(0), // this is a BigNumber
        totalHashes: web3Utils.toBN(0), // this is a BigNumber
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
        hashRateDisplay () {
            /* Initialize big number constants. */
            const bnTril = web3Utils.toBN(1000000000000)
            const bnBil = web3Utils.toBN(1000000000)
            const bnMil = web3Utils.toBN(1000000)
            const bnThou = web3Utils.toBN(1000)

            /* Initialize "decimal" multiplier. */
            const mult = web3Utils.toBN(100) // to one-hundreth

            /* Validate hash rate. */
            if (this.hashRate.isZero())
                return 'n/a'

            /* Initialize a decimal hash rate. */
            // NOTE: 2 orders of magnitude added for decimal calculation.
            let hashRateDecimal = this.hashRate.mul(mult)

            /* Parse the number (based on number of digits). */
            if (hashRateDecimal.gt(bnTril.mul(mult)))
                return (hashRateDecimal.div(bnTril)) / 100.0 + 'T/s'
            else if (hashRateDecimal.gt(bnBil.mul(mult)))
                return (hashRateDecimal.div(bnBil)) / 100.0 + 'B/s'
            else if (hashRateDecimal.gt(bnMil.mul(mult)))
                return (hashRateDecimal.div(bnMil)) / 100.0 + 'M/s'
            else if (hashRateDecimal.gt(bnThou.mul(mult)))
                return (hashRateDecimal.div(bnThou)) / 100.0 + 'k/s'
            else
                return hashRateDecimal / 100.0 + '/s'
        },
        totalHashesDisplay () {
            /* Initialize big number constants. */
            const bnTril = web3Utils.toBN(1000000000000)
            const bnBil = web3Utils.toBN(1000000000)
            const bnMil = web3Utils.toBN(1000000)
            const bnThou = web3Utils.toBN(1000)

            /* Initialize "decimal" multiplier. */
            const mult = web3Utils.toBN(10) // to one-tenth

            /* Validate number of hashes. */
            if (this.totalHashes.isZero())
                return 'n/a'

                /* Initialize a decimal hash rate. */
                // NOTE: 2 orders of magnitude added for decimal calculation.
                let totalHashesDecimal = this.totalHashes.mul(mult)

            /* Parse the number (based on number of digits). */
            if (totalHashesDecimal.gt(bnTril.mul(mult)))
                return (totalHashesDecimal.div(bnTril)) / 10.0 + 'T'
            else if (totalHashesDecimal.gt(bnBil.mul(mult)))
                return (totalHashesDecimal.div(bnBil)) / 10.0 + 'B'
            else if (totalHashesDecimal.gt(bnMil.mul(mult)))
                return (totalHashesDecimal.div(bnMil)) / 10.0 + 'M'
            else if (totalHashesDecimal.gt(bnThou.mul(mult)))
                return (totalHashesDecimal.div(bnThou)) / 10.0 + 'k'
            else
                return totalHashesDecimal.toNumber()
        },
        numLodesDisplay () {
            return numeral(this.numLodes).format('0,0')
        }
    },
    methods: {
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
                        this.stopWorker()

                        /* Give it a sec. */
                        setTimeout(() => {
                            this.minadoAddress = data.address
                            this.minadoChallenge = data.challenge
                            this.minadoDifficulty = data.difficulty
                            this.minadoTarget = data.target

                            /* Start the miner. */
                            this.startWorker()
                        }, 1000)
                    }

                    /* Start mining. */
                    if (action === 'start_mining' && data.tag === this.tag) {
                        /* Validate running instance. */
                        if (!this.isMining) {
                            /* Start the miner. */
                            this.startWorker()
                        }
                    }

                    /* Stop mining. */
                    if (action === 'stop_mining' && data.tag === this.tag) {
                        /* Stop the miner. */
                        this.stopWorker()
                    }
                }
            }

            this.ws.onclose = () => {
                console.log('Connection closed.')

                /* Try to reconnect after 10 seconds. */
                setTimeout(() => {
                    console.log('Trying to re-connect..')

                    /* Re-initialize. */
                    this.init()
                }, 10000)
            }

            /* Initialize change tag listener. */
            ipc.on('changeTag', () => {
                this.changeTag()
            })

            /* Handle (process) exit. */
            process.on('exit', () => {
                console.log('Process exiting .. Stopping miner.')

                /* Stop the CPU miner. */
                this.stopWorker()
            })
        },

        /* Open web browser hyperlink. */
        open (link) {
            this.$electron.shell.openExternal(link)
        },

        startWorker () {
            /* Initialize cross spawn. */
            const spawn = require('cross-spawn')

            /* Set hashing start time. */
            this.hashStartTime = moment().unix()

            /* Spawn new instance. */
            this.ps = spawn('./bin/ministo', [this.minadoChallenge, this.minadoTarget])

            /* Handle (incoming) data. */
            this.ps.stdout.on('data', (data) => {
                // console.log(data.toString())

                /* Parse the incoming data. */
                this.parseData(data)
            })

            /* Handle data errors. */
            this.ps.stderr.on('data', (data) => {
                console.log(`ps stderr: ${data}`)
            })

            /* Handle (worker) close. */
            this.ps.on('close', (code) => {
                if (code !== 0) {
                    console.log(`Process exited with code [ ${code} ]`)
                }
            })
        },

        stopWorker () {
            if (this.ps) {
                /* Terminate the process. */
                this.ps.kill('SIGTERM')

                /* Update display. */
                this.realtime = 'Worker has stopped!'
            }
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

        parseData (_data) {
            /* Validate data. */
            if (!_data) {
                return console.error('The `parseData` you sent is invalid!')
            }

            /* Initialize data. */
            // NOTE: Data arrives as a buffer (from `cross-spawn`).
            const data = _data.toString()

            // console.log('INCOMING DATA', data)

            /* Split the data into lines. */
            const lines = data.split('\n')

            /* Iterate through and parse each line of data. */
            for (let line of lines) {
                /* Validate line and look for command prefix. */
                if (line !== '' && line.slice(0, 2) === '::') {
                    // console.log('THIS IS A COMMAND', data)

                    /* Locate data break. */
                    // Separates the `command` from the `value`.
                    let dataBreak = line.indexOf('::', 2)

                    /* Parse the command. */
                    const cmd = line.slice(2, dataBreak)

                    // console.log('COMMAND', cmd)

                    /* Locate data termination. */
                    let dataTerm = line.indexOf('::', dataBreak + 2)

                    /* Parse the value. */
                    const val = line.slice(dataBreak + 2, dataTerm)

                    // console.log('VALUE', val)

                    /* Handle commands. */
                    switch (cmd) {
                        case 'NOTIFY:HASHES':
                            /* Convert to a big number. */
                            let hashes = web3Utils.toBN(val)

                            /* Add to last hashes. */
                            this.lastHashes = this.lastHashes.add(hashes)

                            /* Add to total hashes. */
                            this.totalHashes = this.totalHashes.add(hashes)

                            let bnSecondsElapsed = web3Utils.toBN(
                                moment().unix() - this.hashStartTime)

                            let hashRate = this.totalHashes.div(bnSecondsElapsed)

                            /* Update hash rate. */
                            this.hashRate = hashRate

                            break
                        case 'NOTIFY:SOLUTION':
                            /* Update real-time data. */
                            this.realtime = `${cmd}\n${val.replace(/:/g, '\n')} (seconds)`

                            /* Parse out the solution. */
                            let solution = val.split(':')[0]

                            /* Validate solution. */
                            if (solution.length === 66) {
                                /* Verify and submit solution. */
                                this.verifyAndSubmit(solution)

                                // FOR TESTING PURPOSES ONLY
                                // if (this.numLodes > 5)
                                //     this.stopWorker()
                            } else {
                                console.error(`[ ${solution} ] failed to validate.`)
                                // TODO Handle error!
                            }

                            break
                        default:
                            /* Update real-time data. */
                            this.realtime = `${cmd}\n${val}`

                            // TODO Add remaining handlers.
                    }
                }
            }
        },

        verifyAndSubmit (_solution) {
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
                console.log(`Submit mined solution [ ${_solution} ] for challenge [ ${this.minadoChallenge} ] with digest [ ${digest} ]`)

                /* Set last hashes. */
                let lastHashes = this.lastHashes.toString()

                /* Reset last hashes. */
                this.lastHashes = web3Utils.toBN(0)

                /* Build package. */
                const pkg = {
                    action: 'submit',
                    // client: 'ministo',
                    // tag: this.tag,
                    // version: this.version,
                    token: '0xf6E9Fc9eB4C20eaE63Cb2d7675F4dD48B008C531', // ZeroGold
                    solution: _solution,
                    lastHashes,
                    digest,
                    challenge: this.minadoChallenge,
                    difficulty: this.minadoDifficulty
                }

                /* Send package. */
                this.ws.send(JSON.stringify(pkg))

                /* Increment number of lodes. */
                // FIXME: Add support for larger (64-bit) lodes.
                this.numLodes++
            } else {
                console.error(`
    Verification Failed!
    --------------------

    Challenge : ${this.minadoChallenge}
    Address   : ${this.minadoAddress}
    Solution  : ${_solution}
    Digest    : ${digestBN}
    Target    : ${this.minadoTarget}
                `)

                /* Stop the miner. */
                this.stopWorker()

                /* Give it a sec. */
                setTimeout(() => {
                    /* Start the miner. */
                    // this.startWorker()

                    /* Re-initiailze foreman. */
                    // this.init()

                    /* Build "authorization" package. */
                    const pkg = {
                        action: 'authorize',
                        client: 'ministo',
                        tag: this.tag,
                        version: this.version
                    }

                    /* Send package. */
                    this.ws.send(JSON.stringify(pkg))
                }, 1000)
            }
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
    font-size: 0.8em;
    font-weight: bold;

    text-align: left;
    // text-align: center;

    background-color: rgba(45, 120, 220, 0.5);
    border: 1pt solid #333;
}

.tag {
    letter-spacing: 5px;
}
</style>
