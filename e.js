const fs = require('fs')
const inquirer = require('inquirer')
const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline');
const { start } = require('repl');
var port;
var sectorArr = []
const parser = new Readline()
SerialPort.list().then(function(ports, err){
    if(ports.length < 1){
        console.log('No USB devices found')}
    else{
        for(var i = 0; i < ports.length; i++){
            if(ports[i].productId == '8037'){
                port = new SerialPort(ports[i].path, { baudRate: 115200 })
                port.write('001')
                port.pipe(parser)
                startPrompt()
            }
        if(err){console.log('err')}
    }
    if(port == undefined || port == null){
        console.log('\nArduino Pro Micro not found')
    }
    }
}
)

function startPrompt() {

}

parser.on('data', function (line){
    if(line.length > 1){
        let lineSplit = line.split(' ');
        if(lineSplit[0] == 'Read' && lineSplit[1] == 'in'){console.log(line)}else
        if(lineSplit[0] == 'Authentication' && lineSplit[1] == 'failed'){console.log(line)}else
        {   let rParse = line.split(':')
            console.log(line)
            sectorArr.push({sector: rParse[0], data: rParse[1]})
            // console.log(sectorArr)
        }
    }
})