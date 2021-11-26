const mongoose = require('mongoose')

const ConfigSchema = new mongoose.Schema({
  deviceId: {
    type: Number,
    required: true,
  },
  greenHouseId: {
    type: String,
    required: true,
  },
  samplingTime: {
    type: Number,
    default: 10,
  },
  date: {
    type: Date,
    default: Date.now,
  }
}, { versionKey: false });

const Config = mongoose.model('Config', ConfigSchema)

module.exports = Config