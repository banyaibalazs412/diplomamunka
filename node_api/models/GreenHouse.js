const mongoose = require('mongoose')

const GreenHouseSchema = new mongoose.Schema({
  name: {
    type: String,
    required: true,
  },
  address: {
    type: String,
  },
  maxColumn: {
    type: Number,
    required: true,
  },
  maxRow: {
    type: Number,
    required: true,
  },
  date: {
    type: Date,
    default: Date.now,
  }
}, { versionKey: false })

const GreenHouse = mongoose.model('GreenHouses', GreenHouseSchema)

module.exports = GreenHouse