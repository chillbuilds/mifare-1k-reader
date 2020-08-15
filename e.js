const fs = require('fs')
const inquirer = require('inquirer')
const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline');
const { start } = require('repl');
var port;
const parser = new Readline({delimiter: '\r\n'})
SerialPort.list().then(function(ports, err){
    if(ports.length < 1){
        console.log('No USB devices found')}
    else{
        for(var i = 0; i < ports.length; i++){
            if(ports[i].productId == '8037' || ports[i].productId == '0043'){
                port = new SerialPort(ports[i].path, { baudRate: 115200 }, function(err){if(err)console.log('\nError accessing COM port\nReason: COM ACCESS DENIED\nPlease quit application\n\n')})
                port.pipe(parser)
                console.log('\nReader connected successfully\n')
                startPrompt(true)
            }
        if(err){console.log('err')}
    }
    if(port == undefined || port == null){
        console.log('\nArduino Pro Micro not found')
    }
    }
}
)

function startPrompt(wait) {
    if(wait == true){
        console.log('Please wait for initialization to complete\n')
    setTimeout(function(){
    inquirer.prompt({
        type: 'list',
        name: 'req',
        message: 'What would you like to do?',
        choices: ['Update Sector','Retrieve All Sector Data', 'Quit Application']
    }).then(function(data){
        if(data.req == 'Update Sector'){
            update()
        }else if(data.req == 'Retrieve All Sector Data'){retrieve()}
        else{quit()}
    })
    }, 2000)}else{
    inquirer.prompt({
        type: 'list',
        name: 'req',
        message: 'What would you like to do?',
        choices: ['Update Sector','Retrieve All Sector Data', 'Quit Application']
    }).then(function(data){
        if(data.req == 'Update Sector'){
            update()
        }else if(data.req == 'Retrieve All Sector Data'){retrieve()}
        else{quit()}
    })
    }
    
}

parser.on('data', function (line){
    console.log(line)
    if(line == 'Retrying data retrieval..'){retrieve()}
    if(line == 'Data written to sector'){startPrompt(false)}
    if(line == 'Data dump was successful'){startPrompt(false)}
})

function update() {
    inquirer.prompt({
        name: 'writeData',
        type: 'input',
        message: 'Enter data (34 character limit per sector)'}).
        then(function(data){
            if(data.writeData.length > 34){
                console.log(`Data limit exeeded by ${data.writeData.length-34}`);update()}
            else{sectorPrompt(data.writeData)}
    })
}

function sectorPrompt(writeData) {
    inquirer.prompt({
        name: 'sector',
        type: 'number',
        message: 'Enter sector to write data to (1 - 15)'
}).then(function(data){
    if(data.sector > 0 && data.sector < 16){
    serialUpdate(writeData, data.sector)}else
    {   console.log('\nProvided sector is out of bounds..\n')
        sectorPrompt(writeData)}
})
}

function serialUpdate(data, sector) {
    port.write(`½${sector}§`)
    port.write(`;${data};•`)
    // startPrompt()
}

function retrieve() {
    port.write('�')
}

function quit() {
    port.close()
    return;
}

// 0123456789abcdefghijklmnopqrstuvwxy