const express = require('express')
const app = express()
const authJwt = require('../authJwt')
const GreenHouse = require('../models/GreenHouse')
const Users = require('../models/Users')
const Device = require('../models/Device')

/**
 * Üvegházak lekérdezése
 */
app.get('/', [authJwt.verifyToken, authJwt.isSuperAdmin], async (req, res) => {
    const greenHouses = await GreenHouse.find()
    
    if (!greenHouses) {
        return res.status(500).send({ error: 'Hiba lépett fel az üvegházak lekérdezése közben!'})
    }

    res.status(200).send(greenHouses)
})

/**
 * Üvegház adatok módosítása
 */
app.put('/',[authJwt.verifyToken, authJwt.isSuperAdmin], async (req, res) => {
    GreenHouse.findByIdAndUpdate(req.body._id, req.body, (err) => {
        if (err) {
          console.error(err)
          return res.status(500).send({error: 'A felhasználó adatainak frissítése sikertelen!'})
        }

        return res.status(200).send({status: 'Sikeresen frissítetted a felhasználó adatait!'})
    })
})

/**
 * Üvegház létrehozása
 */
app.post('/',[authJwt.verifyToken, authJwt.isSuperAdmin], async (req, res) => {
    GreenHouse.create(req.body)
      .then(() => { res.status(200).send({ status: 'Üvegház sikeresen létrehozva!'})})
      .catch((error) => console.error(error))
})

/**
 * Üvegház törlése
 */
app.delete('/',[authJwt.verifyToken, authJwt.isSuperAdmin], async (req, res) => {
    GreenHouse.findByIdAndDelete(req.body.id, (err) => {
      if (err) {
          return res.status(500).send({ error: 'Sikertelen üvegház törlés!'})
      }

      res.status(200).send({ status: 'Sikeresen töröltük az üvegházat!'})
    })
})

/**
 * Lekérdezi az üvegházhoz tartozó maximum oszlop és sor számot
 */
app.get('/maxColAndRow', [authJwt.verifyToken], async (req, res) => {
    const requestedUser = await Users.findById(req.userId)

    if(requestedUser.greenHouseId) {
        const greenHouse = await GreenHouse.findOne({ _id: requestedUser.greenHouseId})
        
        if (greenHouse) {
            const devices = await Device.find({ greenHouseId: greenHouse._id.toString()})  

            return res.status(200).send(
                {
                    maxColumn: greenHouse.maxColumn,
                    maxRow: greenHouse.maxRow,
                    deviceNumber: devices.length
                }
            )
        }

        return res.status(500).send({ error: 'Nincs ilyen üvegház!'})
    }
    
    res.status(500).send({ error: 'Nincs ilyen üvegház!'})
})

/**
 * Üvegházak listája formázva
 */
app.get('/getGreenHouseListForVue',[authJwt.verifyToken, authJwt.isAdminOrSuperAdmin], async (req, res) => {
    const greenHouses = await GreenHouse.find()
    let greenHouseArray = [];

    greenHouses.forEach((greenHouse) => {
        greenHouseArray.push(
        {
          id: greenHouse.id,
          name: greenHouse.name
        }
      )
    })

    res.status(200).send(greenHouseArray)
})

module.exports = app;