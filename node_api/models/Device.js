const mongoose = require('mongoose')

const DeviceSchema = new mongoose.Schema({
  deviceId: {
    type: Number,
    required: true,
  },
  greenHouseId: {
    type: String,
    required: true,
  },
  ipAddress: {
    type: String,
    required: true,
  },
  column: {
    type: Number,
    required: true,
  },
  row: {
    type: Number,
    required: true,
  },
  date: {
    type: Date,
    default: Date.now,
  }
}, { versionKey: false });

const Device = mongoose.model('Device', DeviceSchema)

module.exports = Device