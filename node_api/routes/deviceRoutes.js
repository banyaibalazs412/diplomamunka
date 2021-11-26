const express = require('express')
const app = express()
const authJwt = require('../authJwt')
const Device = require('../models/Device')
const DeviceData = require('../models/DeviceData')
const GreenHouse = require('../models/GreenHouse')
const http = require('http')
const Users = require('../models/Users')
const Config = require('../models/Config')

/**
 * Eszközök lekérdezése
 */
app.get('/',[authJwt.verifyToken] , async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    let devices;
    if (!requestedUser.greenHouseId) {
      return res.status(400).send({ error: 'Hiba lépett fel az eszközök lekérdezése közben!'})
    } else {
        devices = await Device.find({greenHouseId: requestedUser.greenHouseId})
    }

    if (!devices) {
      return res.status(400).send({ error: 'Hiba lépett fel az eszközök lekérdezése közben!'})
    }

    res.status(200).send(devices)
})

/**
 * Eszköz mentése és configuráció létrehozása, ha a mentés sikeres
 */
app.post('/',[authJwt.verifyToken], async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    const devices = await Device.findOne({greenHouseId:  req.body.greenHouseId === '' ? requestedUser.greenHouseId : req.body.greenHouseId,  deviceId: req.body.deviceId})

    if (devices) {
        return res.status(400).send({ error: 'Adatok rögzítése sikertelen! A deviceId már foglalt!'})
    }

    const greenHouseIdFromQuery = req.body.greenHouseId === '' ? requestedUser.greenHouseId : req.body.greenHouseId
    const greenHouse = await GreenHouse.findById(greenHouseIdFromQuery)

    if (!greenHouse || greenHouse.maxColumn < req.body.column || greenHouse.maxRow < req.body.row) {
        return res.status(400).send({
            error: 'Adatok rögzítése sikertelen!' +
                ' A megadott üvegház nem található, vagy nem megfelelő az oszlop és/vagy sorok száma!'
        })
    }

    const newDevice = {
        deviceId: req.body.deviceId,
        greenHouseId: greenHouseIdFromQuery,
        ipAddress: req.body.ipAddress,
        column: req.body.column,
        row: req.body.row,
    }

  const newConfig = {
      deviceId: req.body.deviceId,
      greenHouseId: req.body.greenHouseId === '' ? requestedUser.greenHouseId : req.body.greenHouseId,
  }

  Device.create(newDevice)
    .catch((error) => {
      return res.status(400).send({error: 'Adatok rögzítése sikertelen!'})
  })

  Config.create(newConfig)
    .then(() => { res.status(200).send({ status: 'Adatok rögzítve!'})})
    .catch((error) => {
      return res.status(400).send({error: 'Adatok rögzítése sikertelen!'})
  })
})

/**
 * Eszköz törlése
 */
app.delete('/',[authJwt.verifyToken], async (req, res) => {
    Device.findByIdAndDelete(req.body.id, (err) => {
      if (err) {
        return res.status(400).send({ error: 'Sikertelen eszköz törlés!'})
      }
      res.status(200).send({ status: 'Sikeresen töröltük az eszközt!'})
    })
})

/**
 * Oszlophoz és sorhoz tartozó eszközök lekérdezése
 */
app.get('/deviceList',[authJwt.verifyToken] , async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    if (!requestedUser.greenHouseId) {
        return res.status(400).send({ error: 'Sikertelen eszköz lekérdezés!'})
    }

    const devices = await Device.find({greenHouseId: requestedUser.greenHouseId, column: req.query.column, row: req.query.row})
    const deviceList = []

    devices.forEach(device => {
      deviceList.push(
        {
          _id: device._id.toString(),
          deviceId: device.deviceId,
        })
    });

    res.status(200).send(deviceList)
})

/**
 * Eszközhöz tartozó monitorozott adatok lekérdezése
 */
app.get('/deviceData',[authJwt.verifyToken] , async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    
    if (!requestedUser.greenHouseId) {
       return res.status(400).send({ error: 'Sikertelen eszköz lekérdezés!'})
    }

    const device = await Device.findOne({greenHouseId: requestedUser.greenHouseId, _id:req.query._id})
    let deviceDatas;

    if (req.query.date) {
        const isoDate = new Date(req.query.date)
        let isoDatePlusOneday = new Date(req.query.date)
        isoDatePlusOneday.setDate(isoDatePlusOneday.getDate() + 1)
      
        deviceDatas = await DeviceData.find({greenHouseId: device.greenHouseId, deviceId: device.deviceId,
          date: {$gte: isoDate, $lte: isoDatePlusOneday}}).sort({date: 'desc'})
    } else {
        deviceDatas = await DeviceData.find({greenHouseId: device.greenHouseId, deviceId: device.deviceId}).sort({date: 'desc'})
    }

    res.status(200).send(deviceDatas)
})

/**
 * Mérési igény küldése a mikrovezérlő számára
 */
app.get('/startMeasurement',[authJwt.verifyToken] , async (req, res) => {
    const requestedUser = await Users.findById(req.userId)

    if (!requestedUser.greenHouseId) {
        return res.status(400).send({ error: 'Sikertelen eszköz lekérdezés!'})
    }

    const device = await Device.findOne({greenHouseId: requestedUser.greenHouseId, deviceId:req.query.deviceId})

    if (!device || !device.ipAddress) {
        return res.status(400).send({ error: 'Sikertelen eszköz lekérdezés!'})
    }

    const options = {
        hostname: device.ipAddress,
        path: '/measure',
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
    res.status(200).send({ status: 'Mérési kérvény elküldve!'})
})

/**
 * Mért adatok tárolása
 */
app.post('/measuredData', async (req, res) => {
    console.log(req.body)
    DeviceData.create(req.body)
      .then(() => { res.status(200).send({ status: 'Adatok rögzítve!'})})
      .catch((error) => console.error(error))
})
module.exports = app;