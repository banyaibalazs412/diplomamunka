import Vue from 'vue'
import Vuex from 'vuex'
import Axios from "axios";

Vue.use(Vuex)

export default{
    state: {
        username: null,
        email: null,
        token: null,
        role: null,
        greenHouseId: null,
        isLoggedIn: null,
    },
    mutations: {
        /**
         * Beállítja a felhasználónevet
         * @param state
         * @param username
         */
        setUsername(state, username) {
            state.username = username
        },
        /**
         * Beállítja az email címet
         * @param state
         * @param email
         */
        setEmail(state, email) {
            state.email = email
        },
        /**
         * Beállítja a user jogosultságát
         * @param state
         * @param role
         */
        setRole(state, role) {
            state.role = role
        },
        /**
         * Beállítja a user tokent
         * @param state
         * @param token
         */
        setToken(state, token) {
            state.token = token
        },
        /**
         * Beállítja az üvegház id-t amihez a user tartozik
         * @param state
         * @param greenHouseId
         */
        setGreenHouseId(state, greenHouseId) {
            state.greenHouseId = greenHouseId
        },
        /**
         * Beállítja, hogy a user be van-e jelentkezve
         * @param state
         * @param isLoggedIn
         */
        setIsLoggedIn(state, isLoggedIn) {
            state.isLoggedIn = isLoggedIn
        },
    },
    getters: {
        /**
         * Vissza adja, hogy a user be van-e jelentkezve
         * @param state
         * @returns {null | String}
         */
        isLoggedIn: (state) => {
            return state.isLoggedIn
        },
        /**
         * Vissza adja a felhasználónevet
         * @param state
         * @returns {null | String}
         */
        getUsername: (state) => {
            return state.username
        },
        /**
         * Vissza adja az email címet
         * @param state
         * @returns {null | String}
         */
        getEmail: (state) => {
            return state.email
        },
        /**
         * Vissza adja a user jogosultságát
         * @param state
         * @returns {null | String}
         */
        getRole: (state) => {
            return state.role
        },
        /**
         * Vissza adja a userhez tartozó tokent
         * @param state
         * @returns {null}
         */
        getToken: (state) => {
            return state.token
        },
        /**
         * Vissza adja a userhez tartozó üvegház ID-t
         * @param state
         * @return {null | Number}
         */
        getGreenHouseId: (state) => {
            return state.greenHouseId
        }
    },
    actions: {
        initUser({commit}) {
            if (localStorage.getItem('token'))
            {
                Axios.get('/user/userByToken')
                    .then(({ data }) => {
                        commit('setToken', localStorage.getItem('token'))
                        commit('setEmail', data.email)
                        commit('setRole', data.role)
                        commit('setUsername', data.username)
                        if (data.greenHouseId) {
                            commit('setGreenHouseId', data.greenHouseId)
                        }
                        commit('setIsLoggedIn', true)
                    })
                    .catch((error) => {
                        commit('setEmail', null)
                        commit('setUsername', null)
                        commit('setToken', null)
                        commit('setRole', null)
                        commit('setIsLoggedIn', null)
                        commit('setGreenHouseId', null)
                        localStorage.clear()
                        this.$noty.error(error.response.data.error)
                    })
            }
        },
        /**
         * Kijelentkezést végzi el (Törli az összes user információt)
         * @param commit
         */
        logout({commit}) {
            commit('setEmail', null)
            commit('setUsername', null)
            commit('setToken', null)
            commit('setRole', null)
            commit('setIsLoggedIn', null)
            commit('setGreenHouseId', null)
            localStorage.clear()
        }
    }
}
