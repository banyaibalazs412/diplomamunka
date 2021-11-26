import Vue from 'vue'
import App from './App.vue'
import './registerServiceWorker'
import router from './router'
import store from './store'
import VueNoty from 'vuejs-noty'
import { BootstrapVue, BootstrapVueIcons } from 'bootstrap-vue'
import vSelect from 'vue-select'

import 'vue-select/dist/vue-select.css';
import 'bootstrap-vue/dist/bootstrap-vue.css'
import 'bootstrap/dist/css/bootstrap.css'
import 'vuejs-noty/dist/vuejs-noty.css'
import axios from "axios";

/**
 * Base URL megadása, hogy a backend elérésnek változására, ne kelljen mindenhol módosítani
 * @type {string}
 */
axios.defaults.baseURL = "http://localhost:3000/api"
axios.defaults.headers.common['Access-Control-Allow-Origin'] = '*';
axios.defaults.headers.common['Content-Type'] = 'application/json';
/**
 * Axios lekérésekbe rakjuk bele a tokent
 * @type {string}
 */
const token = localStorage.getItem('token')
/**
 * Ha már létezik, akkor a HTTP requestekbe rakjuk bele a headerbe, mert ezt ellenőrzi majd a backend
 */
if (token) {
  axios.defaults.headers.common['x-access-token'] = token
}

Vue.config.productionTip = false

Vue.component('v-select', vSelect)

Vue.use(BootstrapVue)
Vue.use(BootstrapVueIcons)
Vue.use(VueNoty, {
  timeout: 4000,
  progressBar: true,
  layout: 'bottomRight'
})

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
