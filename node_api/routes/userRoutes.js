const express = require('express')
const app = express()
const config = require('../config.js');
const jwt = require('jsonwebtoken');
const authJwt = require('../authJwt')
const { body, check, validationResult } = require('express-validator');
const GreenHouse = require('../models/GreenHouse');
const Users = require('../models/Users')
const bcrypt = require('bcrypt');
const saltRounds = 10;

/**
 * Lekérdezi az összes adatbázisban lévő felhasználót
 */
app.get('/',[authJwt.verifyToken, authJwt.isAdminOrSuperAdmin], async (req, res) => {
    const requestedUser = await Users.findById(req.userId)
    let users = null

    if (requestedUser.role === 'Admin') {
        users = await Users.find({ greenHouseId: requestedUser.greenHouseId })  
    } else {
        users = await Users.find({ role: { $ne: 'SuperAdmin' }})
    }
  
    const greenHouses = await GreenHouse.find()

    let usersArray = [];
    users.forEach((user) => {
      const greenHouseItem = greenHouses.find((item) => {
        return item.id === user.greenHouseId
      })

      usersArray.push(
        {
          id: user.id,
          username: user.username,
          email: user.email,
          role: user.role,
          greenHouseId: user.greenHouseId,
          greenHouseName: greenHouseItem ? greenHouseItem.name : '',
        }
      )
    })

    res.status(200).send(usersArray);
})

app.get('/userByToken',[authJwt.verifyToken], async (req, res) => {
    const user = await Users.findById(req.userId)

    if (!user) {
        return res.status(400).send({error: 'Hiba történt a felhasználó lekérdezése közben!'})
    }

    res.status(200).send({
      username: user.username,
      email: user.email,
      role: user.role,
    })
})

/**
 * A felhasználó bejelentkezésért felelős
 */
app.post('/login', async (req,res) => {
    const user = await Users.findOne({username: req.body.username})
    if (!user) {
        return res.status(401).send({error: 'Nincs ilyen felhasználónévvel regisztráció!'})
    }

    if((!user.greenHouseId || user.greenHouseId === '') && user.role !== 'SuperAdmin') {
        return res.status(401).send({error: 'Nincs üvegház rendelve a felhasználóhoz. Szóljon egy SuperAdminnak!'})
    }

    bcrypt.compare(
      req.body.password,
      user.password,
      (err, isValid) => {
        if (isValid) {

          const token = jwt.sign({ id: user.id }, config.secret, {
            expiresIn: 86400 // 24 óra
          });
        
          return res.status(200).send( 
              {
                id: user.id,
                username: user.username,
                email: user.email,
                role: user.role,
                token: token,
                greenHouseId: user.greenHouseId,
              }
          )
        }

        return res.status(401).send({
          error: 'Hibás Jelszó!'
        });
      }
    );
})
  
/**
 * A regisztrációt hatja végre. Ha az értékek nem validak, vagy már létezik ilyen felhasználó, akkor hibaüzenetet adunk vissza.
 */
app.post('/register',
    check('password')
      .not()
      .isEmpty()
      .withMessage('A jelszó mező kitöltése kötelező!')
      .trim()
      .escape()
      .isLength({ min: 5 })
      .withMessage('A jelszónak legalább 5 karakter kell legyen!'),
    body('email')
      .not()  
      .isEmpty()
      .withMessage('Az email mező kitöltése kötelező!')
      .trim()
      .escape()
      .isEmail()
      .withMessage('Nem megfellő email formátum!')
      .normalizeEmail(),
    body('username')
      .not()
      .isEmpty()
      .withMessage('A név kitöltése kötelező!')
      .trim()
      .escape(),
  async (req,res) => {

    const errors = validationResult(req);
    if (!errors.isEmpty()) {
        return res.status(400).send({ error: errors.array() });
    }

    Users.findOne({$or: [{username: req.body.username}, {email: req.body.email}]}, (err, result) => {
        if (err) {
            console.error(err)
        }
        
        if (result) {
            return res.status(400).send({ error: [{msg: 'A megadott email cím vagy felhasználónév már foglalt.'}]});
        }
    })

    bcrypt.hash(req.body.password, saltRounds, function(err, hash) {
      if (err) {
        console.error(err)
        return res.status(400).json({ error: [{msg: 'Hiba adódott a regisztráció során.'}]});
      }
      Users.create({
        username: req.body.username,
        email: req.body.email,
        password: hash,
      }).then(user => res.status(200).json(user.username)).catch((error) => console.error(error))
  });
})

/**
 * Felhasználó módosítása
 */
app.put('/',[authJwt.verifyToken, authJwt.isAdminOrSuperAdmin], async (req, res) => {
  const requestedUser = await Users.findById(req.userId)
  const userToUpdate = await Users.findById(req.body.id)

  if (requestedUser.role !== 'SuperAdmin' && requestedUser.greenHouseId !== userToUpdate.greenHouseId) {
      return res.status(400).send({error: 'A felhasználó adatainak frissítése nincs jogosultságod!'})
  }
  
  Users.findByIdAndUpdate(req.body.id, req.body, (err) => {
      if (err) {
          console.error(err)
          return res.status(400).send({error: 'A felhasználó adatainak frissítése sikertelen!'})
      }

      return res.status(200).send({status: 'Sikeresen frissítetted a felhasználó adatait!'})
  })
})

/**
 * Felhasználó törlése
 */
app.delete('/',[authJwt.verifyToken, authJwt.isAdminOrSuperAdmin], async (req, res) => {
  const requestedUser = await Users.findById(req.userId)
  const userToDelete = await Users.findById(req.body.id)

  if (requestedUser.role !== 'SuperAdmin' && requestedUser.greenHouseId !== userToDelete.greenHouseId) {
    return res.status(400).send({error: 'A felhasználó adatainak törlésére nincs jogosultságod!'})
  }

  Users.findByIdAndDelete(req.body.id, (err) => {
      if (err) {
        console.error(err)
        return res.status(400).send({error: 'A felhasználó adatainak törlése sikertelen!'})
      }
      return res.status(200).send({status: 'Sikeresen törölted a felhasználót!'})
  })
})

module.exports = app;