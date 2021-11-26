const mongoose = require('mongoose')

const NoteBoardSchema = new mongoose.Schema({
  name: {
    type: String,
    required: true,
  },
  greenHouseId: {
    type: String,
  },
  title: {
    type: String,
    required: true,
  },
  description: {
    type: String,
    required: true,
  },
  date: {
    type: Date,
    default: Date.now,
  }
}, { versionKey: false })

const NoteBoard = mongoose.model('NoteBoard', NoteBoardSchema)

module.exports = NoteBoard