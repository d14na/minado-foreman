<template>
    <div id="container">
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
                <div>
                    Power Level: <strong class="red--text">{{powerLevel}}</strong>
                </div>

                <div>
                    ...
                </div>

                <div>
                    FAST Connectivity
                </div>
            </div>

            <div class="right-side">
                <div>
                    Hash Rate: <strong class="red--text">{{hashRateDisplay}}</strong>
                </div>

                <div>
                    # Hashes: <strong>{{totalHashesDisplay}}</strong>
                </div>

                <div>
                    Lodes Delivered: <strong class="red--text">{{numLodesDisplay}}</strong>
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

const fs = require('fs')

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
        minadoToken: '',
        minadoAddress: '',
        minadoChallenge: '',
        minadoDifficulty: '',
        minadoTarget: '',

        realtime: 'REAL-TIME ACTIVITY GRAPH',

        hashStartTime: 0,
        hashRate: web3Utils.toBN(0), // this is a BigNumber
        lastHashes: web3Utils.toBN(0), // this is a BigNumber
        totalHashes: web3Utils.toBN(0), // this is a BigNumber
        numLodes: web3Utils.toBN(0), // this is a BigNumber

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
        powerLevel () {
            if (this.minadoDifficulty) {
                return this.minadoDifficulty
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
            return numeral(this.numLodes.toString()).format('0,0')
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
                            this.minadoToken = data.token
                            this.minadoAddress = data.address
                            this.minadoChallenge = data.challenge
                            this.minadoDifficulty = data.difficulty
                            this.minadoTarget = data.target

                            /* Update the command file. */
                            this.updateCmdFile(data)

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

        updateCmdFile (_data) {
            /* Initialize timestamp. */
            const timestamp = moment().unix()

            /* Initialize hardware type. */
            const hwType = 'cpu'

            /* Initialize command line. */
            const cmdLine = `${timestamp} ${_data.token} ${_data.challenge} ${_data.target} ${hwType} ${_data.address}`
            console.log('WROTE CMD LINE TO FILE', cmdLine)

            /* Encode to buffer. */
            const clBuffer = Buffer.from(cmdLine)

            fs.writeFile('./data/cmd', clBuffer, (err) => {
                if (err) throw err

                console.log('The file has been saved!')
            })
        },

        startWorker () {
            /* Set (formatted) date today. */
            const dateToday = moment().format('YYYYMMDD')

            /* Initialize filepath. */
            const filepath = `./data/${dateToday}-${this.minadoToken}.log`

            console.info(`Started monitoring [ ${filepath} ] for changes.`)

            const stats = fs.statSync(filepath)
            const fileSizeInBytes = stats['size']

            /* Initialize log (file) size. */
            let mLogSize = fileSizeInBytes

            /* Initialize token. */
            // const token = '0xf6E9Fc9eB4C20eaE63Cb2d7675F4dD48B008C531'

            // /* Initialize monitor. */
            // const monitor = require('text-file-follower')
            //
            // /* Initialize log monitor. */
            // var logMonitor = monitor(filepath)
            //
            // /* Handle new line (changes). */
            // logMonitor.on('line', (filepath, line) => {
            //     // console.log('Got a new line from ' + filepath + ': ' + line)
            //
            //     /* Parse the log entry. */
            //     this.parseLog(line)
            // })

            /* Initialize file monitoring interval. */
            const fileMonitor = setInterval(() => {
                // console.log('running interval', moment(). unix())

                fs.exists(filepath, (_doesExist) => {
                    // console.log(_doesExist ? 'it\'s there' : 'no passwd!')

                    const stats = fs.statSync(filepath)
                    const fileSizeInBytes = stats['size']

                    console.log('fileSizeInBytes', fileSizeInBytes)

                    if (fileSizeInBytes > mLogSize) {
                        /* Set offset. */
                        let offset = mLogSize

                        /* Update log (file) size. */
                        mLogSize = fileSizeInBytes

                        fs.open(filepath, 'r', (_err, _fd) => {
                            let deltaSize = fileSizeInBytes - offset

                            let deltaData = Buffer.alloc(deltaSize)

                            fs.read(_fd, deltaData, 0, deltaSize, offset, (_err, _bytesRead, _buffer) => {
                                // console.log('READ _err', _err);
                                // console.log('READ _bytesRead', _bytesRead);
                                // console.log('READ _buffer', _buffer);

                                // console.log('DELTA DATA', deltaData.toString())

                                let delta = deltaData.toString()

                                /* Remove trailing line-ending. */
                                // NOTE: We need to split multiple lines later.
                                if (delta.slice(-1) === '\n') {
                                    delta = delta.slice(0, -1)
                                }

                                /* Check for multiple lines. */
                                if (delta.indexOf('\n') !== -1) {
                                    // console.error('OH NO. WE GOTTA SPLIT THIS ONE', deltaSize)

                                    let lines = delta.split('\n')

                                    for (let line of lines) {
                                        // console.log('MULTIPLE LINE', line)

                                        this.parseLog(line)
                                    }
                                } else {
                                    // console.log('JUST A SINGLE LINE', delta)

                                    this.parseLog(delta)
                                }
                            })
                        })
                    }
                })
            }, 500)

            // return

            /* Initialize cross spawn. */
            // const spawn = require('cross-spawn')
            const { spawn } = require('child_process')

            /* Set hashing start time. */
            this.hashStartTime = moment().unix()

            /* Spawn new instance. */
            // NOTE: `pipe` may be required on Windows to prevent hanging.
            this.ps = spawn('./bin/minadod')
            // this.ps = spawn('./bin/minadod', [this.minadoChallenge, this.minadoTarget], { stdio: 'pipe' })
            // this.ps = spawn('./bin/minadod', [this.minadoToken, this.minadoChallenge, this.minadoTarget])
            // this.ps = spawn('./bin/minadod', [this.minadoChallenge, this.minadoTarget], { stdio: 'ignore' })

            this.ps.stderr.on('data', (data) => {
                console.log(`stderr: ${data}`);
            })

            /* Handle (worker) close. */
            this.ps.on('close', (code) => {
                if (code !== 0) {
                    console.log(`Process exited with code [ ${code} ]`)

                    /* Wait a sec, then re-start. */
                    setTimeout(() => {
                        this.startWorker() // TEMP FOR DEVELOPMENT PURPOSES ONLY
                    }, 1000)
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

        parseLog (_line) {
            /* Validate line. */
            if (!_line) {
                return console.error('The line you sent is invalid!')
            }

            // console.log('INCOMING LINE', _line)

            const timestamp = _line.split(' ')[0]
            const challenge = _line.split(' ')[1]
            const solution = _line.split(' ')[2]
            const latestHashes = _line.split(' ')[3]
            const secondsToSolve = _line.split(' ')[4]

            /* Convert to a big number. */
            let hashes = web3Utils.toBN(latestHashes)

            /* Add to last hashes. */
            this.lastHashes = this.lastHashes.add(hashes)

            /* Add to total hashes. */
            this.totalHashes = this.totalHashes.add(hashes)

            let bnSecondsElapsed = web3Utils.toBN(
                moment().unix() - this.hashStartTime)

            let hashRate = this.totalHashes.div(bnSecondsElapsed)

            /* Update hash rate. */
            this.hashRate = hashRate

            /* Validate solution. */
            if (solution.length === 66) {
                /* Update real-time data. */
                this.realtime = `Found a solution: \n${solution}`

                /* Verify and submit solution. */
                this.verifyAndSubmit(solution)
            } else {
                console.error(`[ ${solution} ] failed to validate.`)
                // TODO Handle error!
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

                /* Add full lode. */
                this.numLodes = this.numLodes + this.minadoDifficulty
            } else {
                console.error(`
    Verification Failed!
    --------------------

    Challenge : ${this.minadoChallenge}
    Address   : ${this.minadoAddress}
    Solution  : ${_solution}
    Digest    : ${digestBN.toString(16)}
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
#container {
    color: #eef;
}

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
