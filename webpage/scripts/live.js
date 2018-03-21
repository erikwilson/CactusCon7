#!/usr/bin/env node

const spawn = require('cross-spawn')
const fs = require('fs')
const path = require('path')
const browserify = require('browserify')
const livereload = require('browserify-livereload')
const watchify = require('watchify')
const _ = require('lodash')

const outfile = path.join('bundle.js')

const b = browserify({
  entries: 'joy',
  standalone: 'joy',
  cache: {},
  packageCache: {},
  debug: true,
  plugin: [watchify],
})

b.plugin(livereload, {
  host: 'localhost',
  port: 1337,
  outfile,
})

b.on('update', bundle)
b.on('error', console.log)
b.on('syntax', console.log)

function launch() {
  console.log('launching electron')
  const cmd = path.join(__dirname, 'electron.js')
  const child = spawn(cmd, {detached: false, stdio: 'inherit'})
  child.on('close', () => {
    console.log('electron is done')
    process.exit()
  })
}
launch = _.once(launch)

function bundle() {
  b.bundle()
    .on('error',console.error)
    .pipe(fs.createWriteStream(outfile))
    .on('close',launch)
  console.log(`wrote ${outfile}`)
}

bundle()
