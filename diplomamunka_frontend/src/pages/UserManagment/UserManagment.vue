<template>
  <div class="container-xl">
    <h2 class="text-center my-5" v-text="'Felhasználók módosítása'" />
    <div class="user-managment-wrapper">
      <b-table
        sort-icon-left
        striped
        show-empty
        :busy="!isUserListLoaded"
        :sort-by.sync="sortBy"
        :sort-desc.sync="sortDesc"
        :items="userList"
        :fields="fields"
      >
        <template #empty>
          <div class="text-center" v-text="'Nincs elem a listában'" />
        </template>
        <template #table-busy>
          <div class="text-center my-2">
            <b-spinner class="align-middle my-2"></b-spinner>
            <strong>Betöltés...</strong>
          </div>
        </template>
        <template #cell(edit)="data">
          <span class="icon-wrapper" @click="openModal(data.item)">
            <font-awesome-icon icon="edit"/>
          </span>
          <span class="icon-wrapper" @click="deleteRow(data.item)">
            <font-awesome-icon icon="trash-alt"/>
          </span>
        </template>
      </b-table>
    </div>
    <b-modal v-if="showModal" id="user-edit-modal" centered @hide="closeModal" :title="getModalTitle">
      <b-form-group label="Név:">
        <b-form-input v-model="selectedUserData.username" disabled />
      </b-form-group>
      <b-form-group label="Email:">
        <b-form-input v-model="selectedUserData.email" disabled />
      </b-form-group>
      <b-form-group label="Jogosultság:">
        <v-select v-model="selectedUserData.role" :options="rolesForSelect">
          <template v-slot:no-options>
            <span v-text="'Nem található elem a listában'" />
          </template>
        </v-select>
      </b-form-group>
      <b-form-group v-if="getRole === getRoleSuperAdmin" label="Üvegház:">
        <v-select
          v-model="selectedUserData.greenHouseId"
          :options="greenHouseList"
          label="name"
          :reduce="greenHouseList => greenHouseList.id"
        >
          <template v-slot:no-options>
            <span v-text="'Nem található elem a listában'" />
          </template>
        </v-select>
      </b-form-group>
      <template #modal-footer>
        <b-btn variant="success" @click="updateUser" v-text="'Mentés'"/>
      </template>
    </b-modal>
  </div>
</template>

<script>
import Axios from "axios";
import { roles } from '../../constans/RolesConstant.js'
import { library } from '@fortawesome/fontawesome-svg-core'
import { faEdit, faTrashAlt } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'
import { mapGetters } from "vuex";

library.add(faEdit, faTrashAlt)

export default {
  name: 'UserManagment',
  components: { FontAwesomeIcon },
  data() {
    return {
      /**
       * Jelzi, hogy betöltődött-e az adat
       */
      isUserListLoaded: false,
      /**
       * A módosításhoz kiválasztott user adatai
       */
      selectedUserData: null,
      /**
       * Modal megjelenítése
       */
      showModal: false,
      /**
       * Rendezési oszlop
       */
      sortBy: '',
      /**
       * Rendezési irány
       */
      sortDesc: '',
      /**
       * választható jogosultságok
       */
      rolesForSelect: [
        'User',
        'Admin',
      ],
      /**
       * Felhasználó lista
       */
      userList: [],
      /**
       * Üvegház lista
       */
      greenHouseList: [],
      /**
       * Táblázat mezők
       */
      fields: [
        {
          key: 'id',
          label: 'Id',
          sortable: false
        },
        {
          key: 'username',
          label: 'Felhasználónév',
          sortable: true
        },
        {
          key: 'email',
          label: 'Email',
          sortable: true
        },
        {
          key: 'role',
          label: 'Jogosultság',
          sortable: true
        },
        {
          key: 'greenHouseName',
          label: 'Üvegház Neve',
          sortable: false
        },
        {
          key: 'edit',
          label: 'Módosítás',
          sortable: false
        },
      ],
    }
  },
  created() {
    this.initUsers()
    this.initGreenHouseList()
  },
  computed: {
    ...mapGetters({
      getRole: 'getRole',
    }),
    /**
     * Vissza adja a modal fejlécének szövegét
     */
    getModalTitle() {
      return this.selectedUserData.username + ' (' + this.selectedUserData.email + ')'
    },
    /**
     * Vissza adja a SuperAdmin jogosultságot
     */
    getRoleSuperAdmin() {
      return roles.ROLE_SUPERADMIN
    },
  },
  methods: {
    /**
     * Lekérdezi a felhasználókat
     */
    initUsers() {
      this.isUserListLoaded = false
      Axios.get('/user')
        .then(({ data }) => {
          this.userList = data
        })
        .catch( error  => {
          console.error(error)
        })
        .finally(() => {
          this.isUserListLoaded = true
        })
    },
    /**
     * Lekérdezi az üvegházakat
     */
    initGreenHouseList() {
      Axios.get('/greenHouses/getGreenHouseListForVue')
        .then(({ data }) => {
          this.greenHouseList = data
        })
        .catch( error  => {
          this.$noty.error(error.response.data.error)
        })
    },
    /**
     * Modal megnyitása a módosításhoz
     */
    openModal(userData) {
      this.showModal = true
      this.selectedUserData = userData
      this.$nextTick(function () {
        this.$bvModal.show('user-edit-modal')
      })
    },
    /**
     * Modal bezárása és adat ürítés
     */
    closeModal() {
      this.showModal = false
      this.selectedUserData = null
      this.$nextTick(() => {
        this.$bvModal.hide('user-edit-modal')
      })
    },
    /**
     * User törlése
     */
    deleteRow(userData) {
      Axios.delete('/user', {data:{ id: userData.id }})
        .then(({ data }) => {
          this.$noty.success(data.status)
          this.initUsers()
        })
        .catch((error) => {
          this.$noty.error(error.response.data.error)
        })
    },
    /**
     * User adatainak frissítése
     */
    updateUser() {
      Axios.put('/user', this.selectedUserData)
        .then(({ data }) => {
          this.$noty.success(data.status)
        })
        .catch((error) => {
          this.$noty.error(error.response.data.error)
        })
        .finally(() => {
          this.initUsers()
          this.closeModal()
        })
    },
  }
}
</script>

<style lang="scss" src="./UserManagment.scss" scoped />
