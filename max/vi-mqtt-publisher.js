const maxApi = require('max-api');
const mqtt = require('mqtt');

let client;

maxApi.addHandler('connect', (url) => {
    client = mqtt.connect(url);

    client.on('connect', () => {
        maxApi.outlet('connected');
        console.log('connected');
    })
});

maxApi.addHandler('publish', (topic, value) => {
    client.publish(topic, value.toPrecision(6));
    console.log(topic, value);
    maxApi.outlet(value);
});

