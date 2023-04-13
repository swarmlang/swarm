#!/usr/bin/env node
// Import libraries
const fs = require('fs')
const binn = require('./binn.js')

// Make sure we got the binary file as an argument to this script
const args = process.argv.slice(2)
if ( args.length < 1 ) {
    console.error('Missing input file.')
    process.exit(1)
}

// Load the raw binary data
const binaryFile = args[0]
const rawBuffer = fs.readFileSync(binaryFile)

// Skip the first 4 bytes, since SBI contains an identification header
const sbiBuffer = rawBuffer.slice(4, rawBuffer.length)

// Parse the binary const name map from binary_const.h. Yes this is kinda hacky.
const binaryConstHeader = fs.readFileSync(`${__dirname}/../src/vm/walk/binary_const.h`).toString('utf-8')
const binaryConstFieldsArr = binaryConstHeader.trim()
                                        .split("\n")
                                        .filter(Boolean)
                                        .filter(x => x.startsWith('#define BC_'))
                                        .map(x => x.split(' ').slice(1))

const binaryConstFields = {}
for ( const field of binaryConstFieldsArr ) {
    binaryConstFields[field[1]] = field[0]
}

// A helper for replacing the names
const walk = (obj, callback) => {
    callback(obj)
    if ( Array.isArray(obj) ) {
        for ( const item of obj ) {
            walk(item, callback)
        }
    } else if ( typeof obj === 'object' && obj ) {
        for ( const key in obj ) {
            walk(obj[key], callback)
        }
    }
}

// Decode the binary into a JS object
c = 0  // a stupid hack
const obj = binn.decode(sbiBuffer)

// Replace all the binary constants w/ their const names for readability
walk(obj, x => {
    if ( !Array.isArray(x) && typeof x === 'object' && x ) {
        for ( const key in x ) {
            x[binaryConstFields[key]] = x[key]
            delete x[key]
        }
    }
})

// Dump the object for inspection
console.log(JSON.stringify(obj, null, 4))
