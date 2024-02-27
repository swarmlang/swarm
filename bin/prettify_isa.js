#!/usr/bin/env node
// Import libraries
const fs = require('fs')

// Make sure we got the binary file as an argument to this script
const args = process.argv.slice(2)
if ( args.length < 1 ) {
    console.error('Missing input file.')
    process.exit(1)
}

// Load the raw binary data
const binaryFile = args[0]
const rawBuffer = fs.readFileSync(binaryFile)
const isaString = rawBuffer.toString('utf8')

const prettifyInstructions = (instrs) => {
    let ilevel = 0
    const prettified = instrs
        .map(x => {
            const parts = x.split('<')
            return {
                inst: parts[0],
                rest: parts.slice(1).join('<').slice(0, -1),
                orig: x,
            }
        })
        .map(x => {
            if ( x.inst.toLowerCase() === 'call0' || x.inst.toLowerCase() === 'call1' ) {
                x.inst = 'CALL'
            }

            if ( x.inst.toLowerCase() === 'return0' || x.inst.toLowerCase() === 'return1' ) {
                x.inst = 'RETURN'
            }

            return {...x}
        })
        .map(x => {
            const topLevelArgs = []
            let acc = ''
            let nestLevel = 0
            for ( const char of x.rest ) {
                if ( char === ',' && !nestLevel ) {
                    topLevelArgs.push(acc)
                    acc = ''
                    nestLevel = 0
                    continue;
                }

                if ( char === '<' ) {
                    nestLevel += 1
                }

                if ( char === '>' ) {
                    nestLevel -= 1
                }

                acc += char
            }
            topLevelArgs.push(acc);

            return {...x, topLevelArgs: topLevelArgs.map(x => x.trim()).filter(Boolean)}
        })
        .map(x => {

            x.topLevelArgs = x.topLevelArgs
                .map(a => {
                    if ( a.toLowerCase().startsWith('location<') ) {
                        return '$' + a.slice('location<'.length, -1)
                    }

                    if ( a.toLowerCase().startsWith('numberreference<') ) {
                        return a.slice('numberreference<'.length, -1)
                    }

                    if ( a.toLowerCase().startsWith('typereference<primitive<') ) {
                        return 'p:' + a.slice('typereference<primitive<'.length, -2)
                    }

                    if ( a.toLowerCase().startsWith('booleanreference<') ) {
                        return a.slice('booleanreference<'.length, -1)
                    }

                    return a
                })

            return x
        })
        .map(x => {
            let pretty = x.inst.toLowerCase() + ' ' + x.topLevelArgs.join(' ')

            if ( x.inst.toLowerCase() === 'assignvalue' ) {
                pretty = x.topLevelArgs[0] + ' <- ' + x.topLevelArgs[1]
            }

            if ( x.inst.toLowerCase() === 'assigneval' ) {
                pretty = x.topLevelArgs[0] + ' <- ' + prettifyInstructions([x.topLevelArgs[1]])[0]
            }

            return {...x, pretty}
        })
        .map(x => {
            // Add some indentation for clarity
            if ( x.inst.toLowerCase() === 'return' ) {
                ilevel -= 1
            }

            const indent = Array(ilevel).fill('    ').join('')
            x.pretty = indent + x.pretty

            if ( x.inst.toLowerCase() === 'beginfn' ) {
                ilevel += 1
            }

            if ( x.inst.toLowerCase() === 'return' ) {
                x.pretty += '\n'
            }

            return x
        })
        .map(x => x.pretty)



    return prettified
}

let prettified = isaString
    .trim()
    .split('\n')
    .filter(Boolean)

prettified = prettifyInstructions(prettified)


console.log(prettified.join('\n'))
