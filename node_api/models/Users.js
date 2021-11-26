const mongoose = require('mongoose')

const UsersSchema = new mongoose.Schema({
  username: {
    type: String,
    required: true,
  },
  email: {
    type: String,
  },
  password: {
    type: String,
    required: true,
  },
  role: {
    type: String,
    default: 'User',
  },
  greenHouseId: {
    type: String,
  },
  date: {
    type: Date,
    default: Date.now,
  }
}, { versionKey: false })

const Users = mongoose.model('Users', UsersSchema)

module.exports = Users