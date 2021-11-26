const jwt = require('jsonwebtoken')
const config = require('./config.js')
const User = require('./models/Users')

verifyToken = (req, res, next) => {
  let token = req.headers['x-access-token']

  if (!token) {
    return res.status(403).send({ message: 'Hiba lépett fel' })
  }

  jwt.verify(token, config.secret, (err, decoded) => {
    if (err) {
      return res.status(401).send({ message: 'Bejelentkezés szükséges!' })
    }
    req.userId = decoded.id
    next()
  })
}

isAdmin = (req, res, next) => {
  User.findById(req.userId).exec((err, user) => {
    if (err) {
      res.status(500).send({ message: err })
      return
    }

    if(user.role !== 'Admin') {
      res.status(403).send({ message: 'Admin jogkör szükséges!' })
      return
    }
    next()
  })
}

isSuperAdmin = (req, res, next) => {
  User.findById(req.userId).exec((err, user) => {
    if (err) {
      res.status(500).send({ message: err })
      return;
    }

    if(user.role !== 'SuperAdmin') {
      res.status(403).send({ message: 'SuperAdmin jogkör szükséges!' })
      return
    }
    next()
  })
}

isAdminOrSuperAdmin = (req, res, next) => {
  User.findById(req.userId).exec((err, user) => {
    if (err) {
      res.status(500).send({ message: err })
      return;
    }

    if(user.role !== 'SuperAdmin' && user.role !== 'Admin') {
      res.status(403).send({ message: 'Admin vagy SuperAdmin jogkör szükséges!' })
      return;
    }
    next();
  });
};

const authJwt = {
  verifyToken,
  isAdmin,
  isSuperAdmin,
  isAdminOrSuperAdmin,
};
module.exports = authJwt