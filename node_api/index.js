const express = require('express')
const app = express()
const mongoose = require('mongoose')
const bodyParser = require('body-parser')
const cors = require("cors");

const userRoutes = require('./routes/userRoutes')
const greenHouseRoutes = require('./routes/greenHouseRoutes')
const deviceRoutes = require('./routes/deviceRoutes')
const noteBoardRoutes = require('./routes/noteBoardRoutes')
const configRoutes = require('./routes/configRoutes')

const connectionString = 'mongodb+srv://balazs:jelszo@test.nkkzu.mongodb.net/garden_monitoring_app?retryWrites=true&w=majority'

app.use(bodyParser.json())
app.use(bodyParser.urlencoded({ extended: true }))

app.use(cors({
  origin: ['http://localhost:8080','172.16.15.46'],
}))

app.use(function(req, res, next) {
  res.header(
      'Access-Control-Allow-Headers',
      'x-access-token, Origin, Content-Type, Accept'
  );
  next();
})

mongoose.connect(connectionString)

const db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error: '))
db.once('open', function () {
  console.log('Connected successfully')
});

app.use('/api/user', userRoutes)
app.use('/api/greenHouses', greenHouseRoutes)
app.use('/api/device', deviceRoutes)
app.use('/api/noteBoard', noteBoardRoutes)
app.use('/api/config', configRoutes)

app.listen(3000, () => {
  console.log('Server is running at port 3000')
});
