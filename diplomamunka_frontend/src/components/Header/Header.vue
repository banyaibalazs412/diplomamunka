<template>
  <div>
    <b-navbar class="navbar-wrapper" toggleable="lg" type="dark" variant="success">
      <router-link class="navbar-div-item navbar-link" :to="{ name: 'Home'}" >
        <b-navbar-brand v-text="'Főoldal'"/>
      </router-link>
      <b-navbar-toggle target="nav-collapse"></b-navbar-toggle>
      <!-- Nav-bar menüpontok -->
      <b-collapse id="nav-collapse" is-nav right class="navbar-dropdown-wrapper navbar-link collapse-padding">
        <b-navbar-nav v-if="isLoggedIn" class="navbar-right-wrapper">
          <router-link :to="{ name: 'NoteBoard'}" >
            <div v-if="!isUserSuperAdmin" v-text="'Üzenőfal'" class="navbar-div-item collapse-padding-top-item"/>
          </router-link>
          <div v-if="!isUserSuperAdmin" class="separator" />
          <b-nav-item-dropdown v-if="!isUserSuperAdmin" class="navbar-dropdown query-div" right>
            <template #button-content>
              <div v-text="'Lekérdezés'" class="d-inline-block modify-btn " />
            </template>
            <b-dropdown-item>
              <router-link :to="{ name: 'DeviceData'}" >
                <div v-text="'Eszköz adatok lekérdezése'" />
              </router-link>
            </b-dropdown-item>
          </b-nav-item-dropdown>
          <div v-if="isUserAdmin" class="separator" />
          <b-nav-item-dropdown v-if="isUserSuperAdmin || isUserAdmin" class="navbar-dropdown" right>
            <template #button-content>
              <div v-text="'Módosítás'" class="d-inline-block modify-btn" />
            </template>
            <b-dropdown-item v-if="isUserSuperAdmin || isUserAdmin">
              <router-link :to="{ name: 'UserManagment'}" >
                <div v-text="'Felhasználók módosítása'" />
              </router-link>
            </b-dropdown-item>
            <b-dropdown-item v-if="isUserSuperAdmin">
              <router-link :to="{ name: 'GreenHouseManagment'}" >
                <div v-text="'Üvegházak módosítása'" />
              </router-link>
            </b-dropdown-item>
            <b-dropdown-item v-if="isUserAdmin">
              <router-link :to="{ name: 'DeviceManagment'}" >
                <div v-text="'Eszközök módosítása'" />
              </router-link>
            </b-dropdown-item>
            <b-dropdown-item v-if="isUserAdmin">
              <router-link :to="{ name: 'ConfigManagment'}" >
                <div v-text="'Eszközök konfiguráció módosítása'" />
              </router-link>
            </b-dropdown-item>
          </b-nav-item-dropdown>
          <div class="separator" />
          <div v-text="getUsername" class="username-text"/>
          <div class="separator" />
          <div @click="logoutUser" v-text="'Kijelentkezés'" class="navbar-div-item collapse-padding"/>
        </b-navbar-nav>
      </b-collapse>
    </b-navbar>
  </div>
</template>

<script>
import { mapActions, mapGetters } from 'vuex'
import router from '../../router'
import { roles } from '../../constans/RolesConstant.js'

export default {
  name: 'Header',
  computed: {
    ...mapGetters({
      isLoggedIn: 'isLoggedIn',
      getUsername: 'getUsername',
      getRole: 'getRole',
    }),
    /**
     * Vissza adja, hogy a belépett user SuperAdmin-e
     *
     * @return Boolean
     */
    isUserSuperAdmin() {
      return this.getRole === roles.ROLE_SUPERADMIN
    },
    /**
     * Vissza adja, hogy a belépett user Admin-e
     *
     * @return Boolean
     */
    isUserAdmin() {
      return this.getRole === roles.ROLE_ADMIN
    },
  },
  methods: {
    ...mapActions({
      logout: 'logout'
    }),
    /**
     * User kijelentkeztetés, majd átirányítás a login oldalra
     */
    logoutUser(){
      this.logout()
      router.push( { name: 'Login'})
    },
  },
}
</script>

<style lang="scss" src="./Header.scss" scoped />
