const express = require('express')
const Users = require('../models/Users')
const app = express()
const authJwt = require('../authJwt')
const Config = require('../models/Config')
const Device = require('../models/Device')
const http = require('http')

/**
 * Eszköz konfigurációk lekérdezése user tokennel
 */
app.get('/', [authJwt.verifyToken], async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    const configItem = await Config.findOne({greenHouseId: requestedUser.greenHouseId, deviceId: req.query.deviceId})

    if(!configItem) {
        return res.status(400).send({ error: 'Nem található az adott eszköz konfiguráció!'})
    }

    res.status(200).send(configItem)
})

/**
 * Eszköz konfigurációk lekérdezése user token nélkül (mikrovezérlő számára)
 */
app.get('/deviceGetData', async (req, res) => {
    const configItem = await Config.findOne({greenHouseId: req.query.greenHouseId, deviceId: req.query.deviceId})

    console.log(req.query)
    if(!configItem) {
        return res.status(400).send({ error: 'Nem található az adott eszköz konfiguráció!'})
    }

    res.status(200).send({config: configItem.samplingTime})
})

/**
 * Eszköz konfiguráció módosítása
 */
app.post('/',[authJwt.verifyToken], async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    const device = await Device.findOne({ deviceId: req.body.deviceId, greenHouseId: req.body.greenHouseId})

    if (!device) {
        return res.status(400).send({ error: 'Sikertelen eszköz lekérdezés!'})
    }

    Config.findByIdAndUpdate(req.body._id, req.body, (err) => {
        if (err) {
            return res.status(400).send({error: 'Az eszköz konfiguráció módosítása sikertelen!'})
        }

        const options = {
              hostname: device.ipAddress,
              path: '/config',
          }

        var espreq = http.get(options, function(response) {
            console.log('STATUS: ' + response.statusCode);
            console.log('HEADERS: ' + JSON.stringify(response.headers));

            response.on('data', function(chunk) {
                console.log('succes')
            }).on('end', function() {
                console.log('end')
            }).on('error', function(e) {
                console.log('ERROR: ' + e.message);
                return res.status(400).send({ error: 'Sikertelen eszköz lekérdezés!'})
            });

        });

        espreq.end();

        res.status(200).send({status: 'Sikeresen frissítetted az eszköz konfigurációkat!'})
    })

})

module.exports = app