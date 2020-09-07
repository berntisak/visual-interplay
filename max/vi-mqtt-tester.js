const mqtt = require('mqtt');

url = 'mqtt://192.168.0.193';

let client;

client = mqtt.connect(url);
client.on('connect', () => {
    console.log('connected');
})

function randPos() {
    var val = Math.random()  
    client.publish("/point/0/pos/value", val.toPrecision(6));
    console.log("Pos: ", val);
}

function randSize() {
    var val = Math.random()
    client.publish("/point/0/size/value", val.toPrecision(6));
    console.log("Size: ",val);
}

setInterval(randSize, 1200);
setInterval(randPos, 3000);

