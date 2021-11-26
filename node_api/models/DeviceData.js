const mongoose = require('mongoose')

const DeviceDataSchema = new mongoose.Schema({
  deviceId: {
      type: Number,
  },
  greenHouseId: {
      type: String,
  },
  laserDistance: {
      type: Number,
  },
  sonicDistance: {
      type: Number,
  },
  temperature: {
      type: Number,
  },
  humidity: {
      type: Number,
  },
  weight: {
      type: Number,
  },
  moisture: {
      type: Number,
  },
  colorRed: {
      type: Number,
  },
  colorGreen: {
      type: Number,
  },
  colorBlue: {
      type: Number,
  },
  date: {
    type: Date,
    default: Date.now,
  }
}, { versionKey: false })

const DeviceData = mongoose.model('DeviceData', DeviceDataSchema)

module.exports = DeviceData