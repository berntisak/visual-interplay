const mqtt = require('mqtt');

let client;

/*
maxApi.addHandler('connect', (url) => {
    client = mqtt.connect(url);

    client.on('connect', () => {
        maxApi.outlet('connected');
        console.log('connected');
    })
});

maxApi.addHandler('publish', (topic, value) => {
    client.publish(topic, value.toString());
    console.log(topic, value);
    maxApi.outlet(value);
});
*/



url = 'mqtt://192.168.0.193';
client = mqtt.connect(url);
client.on('connect', () => {
    console.log('connected');
})

function randPos() {
    var val = Math.random()  
    client.publish("/point/0/pos/value", val.toString());
    console.log("Pos: ", val);
}

function randSize() {
    var val = Math.random()
    client.publish("/point/0/size/value", val.toString());
    console.log("Size: ",val);
}

setInterval(randSize, 1200);
setInterval(randPos, 3000);

