<template>
  <div class="container text-center">
    <b-card bg-variant="succes" class="input-group-wrapper mx-auto mt-5">
      <h3 class="pt-3">Regisztráció</h3>
      <div class="my-5">
        <b-form-group label="Felhasználónév*:">
          <b-form-input
            v-model="username"
            class="input-field mx-auto mb-3"
            placeholder="Felhasználónév*"
          />
        </b-form-group>
        <b-form-group label="E-mail cím:*">
          <b-form-input
            v-model="email"
            type="email"
            class="input-field mx-auto mb-3"
            placeholder="E-mail cím*"
          />
        </b-form-group>
        <b-form-group label="Jelszó*:" >
          <b-form-input
            v-model="password"
            type="password"
            class="input-field mx-auto"
            placeholder="Jelszó*"
          />
        </b-form-group>
        <b-form-group label="Jelszó ismét*:">
          <b-form-input
            v-model="passwordRepeated"
            type="password"
            class="input-field mx-auto"
            placeholder="Jelszó ismét*"
          />
        </b-form-group>
        <div class="mt-5">
          <b-button
            class="text-center"
            variant="success"
            @click="register"
            v-text="'Regisztráció'"
          />
        </div>
      </div>
    </b-card>
  </div>
</template>

<script>
import Axios from "axios";

export default {
  name: 'Register',
  data() {
    return {
      /**
       * Felhasználónév
       */
      username: '',
      /**
       * E-mail cím
       */
      email: '',
      /**
       * Jelszó
       */
      password: '',
      /**
       * Jelszó ismételten
       */
      passwordRepeated: '',
    }
  },
  methods: {
    /**
     * Regisztráció
     */
    register() {
      var re = /\S+@\S+\.\S+/;

      if(this.password !== this.passwordRepeated) {
        this.$noty.error('A megadott jelszavak különböznek!')
        return
      }

      if(!re.test(this.email)) {
        this.$noty.error('Az email cím helytelen!')
        return
      }

      Axios.post('/user/register',
        {
          username: this.username,
          email: this.email,
          password: this.password,
        }
      )
        .then(() => {
          this.$noty.success('Sikeres regisztráció')
          this.$router.push({ name: 'Login' })
        })
        .catch((error) => {
          error.response.data.error.forEach(err => {
            this.$noty.error(err.msg)
          })
        })
    },
  }
}
</script>

<style lang="scss" src="./Register.scss" scoped />
