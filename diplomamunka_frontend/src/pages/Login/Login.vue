<template>
  <div class="container text-center">
    <b-card bg-variant="succes" class="input-group-wrapper mx-auto mt-5">
      <h3 class="pt-3">Bejelentkezés </h3>
      <div class="my-5">
        <b-form-group label="Felhasználónév:">
          <b-form-input
              v-model="username"
              class="input-field mx-auto mb-3"
              placeholder="Felhasználónév"
              @keydown.enter="login"
          />
        </b-form-group>
        <b-form-group label="Jelszó:">
          <b-form-input
              v-model="password"
              type="password"
              class="input-field mx-auto"
              placeholder="Jelszó"
              @keydown.enter="login"
          />
        </b-form-group>
        <div class="mt-5">
          <b-button
              class="text-center"
              variant="success"
              @click="login"
              v-text="'Bejelentkezés'"
          />
        </div>
        <b-button
          class="text-center mt-3 btn-dark"
          variant="success"
          @click="goToRegister"
          v-text="'Regisztráció'"
        />
      </div>
    </b-card>
  </div>
</template>

<script>
import Axios from 'axios'
import {mapActions, mapGetters, mapMutations} from 'vuex'


export default {
  name: "Login",
  data() {
    return {
      /**
       * Felhasználónév
       */
      username: '',
      /**
       * Jelszó
       */
      password: '',
    }
  },
  computed: {
    ...mapGetters( {
      getUsername: 'getUsername',
    }),
  },
  methods: {
    ...mapMutations({
      setUsername: 'setUsername',
      setEmail: 'setEmail',
      setRole: 'setRole',
      setToken: 'setToken',
      setIsLoggedIn: 'setIsLoggedIn',
      setGreenHouseId: 'setGreenHouseId',
    }),
    ...mapActions({
      initUser: 'initUser',
    }),
    /**
     * Bejelentkezés
     */
    login() {
      Axios.post('/user/login',
          {
            username: this.username,
            password: this.password
          })
      .then(({ data }) => {
        this.setToken(data.token)
        this.setUsername(data.username)
        this.setEmail(data.email)
        this.setRole(data.role)
        this.setIsLoggedIn(true)
        if (data.greenHouseId) {
          this.setGreenHouseId(data.greenHouseId)
        }
        localStorage.setItem('token', data.token)
        localStorage.setItem('role', data.role)
        localStorage.setItem('greenHouseId', data.greenHouseId ? data.greenHouseId : null)
        this.$router.go()
        this.$noty.success("Sikeresen bejelentkeztél!")
      })
      .catch( error  => {
        this.$noty.error(error.response.data.error)
      })
      .finally(() => {
        this.initUser()
      })
    },
    /**
     * Átirányítás a regisztrációs oldalra
     */
    goToRegister() {
      this.$router.push({ name: 'Register' })
    }
  },
}
</script>

<style lang="scss" src="./Login.scss" scoped />
