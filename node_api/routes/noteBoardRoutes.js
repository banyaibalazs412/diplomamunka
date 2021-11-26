const express = require('express')
const app = express()
const authJwt = require('../authJwt')
const NoteBoard = require('../models/NoteBoard');
const Users = require('../models/Users')

/**
 * Üzenőfal bejegyzésel kelérdezése
 */
app.get('/',[authJwt.verifyToken] , async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    const noteBoardItems = await NoteBoard.find({greenHouseId: requestedUser.greenHouseId}).sort({date: 'desc'})

    res.status(200).send(noteBoardItems)
})

/**
 * Üzenőfal bejegyzés készítése
 */
app.post('/',[authJwt.verifyToken], async (req, res) => {
    const requestedUser = await Users.findById(req.userId)

    const newNoteBoardComment = {
        name: requestedUser.username,
        greenHouseId: requestedUser.greenHouseId,
        title: req.body.title,
        description: req.body.description,
    }

    NoteBoard.create(newNoteBoardComment)
      .then(() => { res.status(200).send({ status: 'Adatok rögzítve!'})})
      .catch((error) => {
        console.error(error)
        res.status(400).send({error: 'Adatok rögzítése sikertelen!'})
    })
})

module.exports = app;