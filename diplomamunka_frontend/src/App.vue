<template>
  <div id="app">
      <Header />
    <router-view/>
  </div>
</template>

<script>
import Header from "./components/Header/Header.vue";
import { mapActions } from "vuex";


export default {
  name: 'App',
  components: { Header },
  created() {
    this.initUser()
    document.title = "Növény monitorozó"
  },
  methods: {
    ...mapActions({
      initUser: 'initUser',
    })
  }
}
</script>
<style lang="scss">
body {
  min-height: 900px;
  width: 100%;

  @media only screen and (max-width: 991px) {
    min-height: auto;
  }
}
</style>
