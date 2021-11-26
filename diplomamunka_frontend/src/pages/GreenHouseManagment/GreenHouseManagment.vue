<template>
  <div class="container-xl">
    <h2 class="text-center my-5" v-text="'Üvegházak módosítása'" />
    <div class="w-100 text-right pb-5">
      <b-button v-text="'Új üvegház létrehozása'" @click="openEmptyModal" />
    </div>
    <div class="green-house-managment-wrapper">
    <b-table
      sort-icon-left
      striped
      show-empty
      :busy="!isGreenHouseListLoaded"
      :sort-by.sync="sortBy"
      :sort-desc.sync="sortDesc"
      :items="greenHouseList"
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
    <b-modal
      v-if="showModal"
      id="green-house-edit-modal"
      centered @hide="closeModal"
      :title="selectedGreenHouse._id ? selectedGreenHouse._id : 'Új üvegház létrehozása'"
    >
      <b-form-group label="Név:">
        <b-form-input v-model="selectedGreenHouse.name" />
      </b-form-group>
      <b-form-group label="Cím:">
        <b-form-input v-model="selectedGreenHouse.address" />
      </b-form-group>
      <b-form-group label="Oszlopok száma">
        <b-form-input type="number" v-model="selectedGreenHouse.maxColumn" />
      </b-form-group>
      <b-form-group label="Sorok száma">
        <b-form-input type="number" v-model="selectedGreenHouse.maxRow" />
      </b-form-group>
      <template #modal-footer>
        <b-btn variant="success" @click="updateOrCreateGreenHouse" v-text="'Mentés'"/>
      </template>
    </b-modal>
  </div>
</template>

<script>
import Axios from "axios";
import { library } from '@fortawesome/fontawesome-svg-core'
import { faEdit, faTrashAlt } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'

library.add(faEdit, faTrashAlt)

export default {
  name: 'GreenHouseManagment',
  components: { FontAwesomeIcon },
  data() {
    return {
      /**
       * Betöltött-e az üvegház lista
       */
      isGreenHouseListLoaded: false,
      /**
       * Üvegház lista
       */
      greenHouseList: [],
      /**
       * Kiválasztott üvegház
       */
      selectedGreenHouse: {},
      /**
       * Lista rendezés
       */
      sortBy: '',
      /**
       * Lista rendezés
       */
      sortDesc: '',
      /**
       * Modal látható-e
       */
      showModal: false,
      /**
       * Táblázat mezők
       */
      fields: [
        {
          key: '_id',
          label: 'Id',
          sortable: false,
        },
        {
          key: 'name',
          label: 'Név',
          sortable: true,
        },
        {
          key: 'address',
          label: 'Cím',
          sortable: false,
        },
        {
          key: 'maxColumn',
          label: 'Oszlopok száma',
          sortable: false,
        },
        {
          key: 'maxRow',
          label: 'Sorok száma',
          sortable: false,
        },
        {
          key: 'date',
          label: 'Létrehozás dátuma',
          sortable: true,
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
    this.initGreenHouseList()
  },
  methods: {
    /**
     * Üvegház lista inicializálás
     */
    initGreenHouseList() {
      this.isGreenHouseListLoaded = false
      Axios.get('/greenHouses')
        .then(({ data }) => {
          this.greenHouseList = data
        })
        .catch( error  => {
          this.$noty.error(error.response.data.error)
        })
        .finally(() => {
          this.isGreenHouseListLoaded = true
        })
    },
    /**
     * Modal megnyitása a módosításhoz
     */
    openModal(greenHouseData) {
      this.showModal = true
      this.selectedGreenHouse = greenHouseData
      this.$nextTick(function () {
        this.$bvModal.show('green-house-edit-modal')
      })
    },
    /**
     * Új elem modál létrehozása
     */
    openEmptyModal() {
      this.showModal = true
      this.selectedGreenHouse = {
        name: '',
        address: '',
        maxColumn: 1,
        maxRow: 1,
      }
      this.$nextTick(function () {
        this.$bvModal.show('green-house-edit-modal')
      })
    },
    /**
     * Modal bezárása és adat ürítés
     */
    closeModal() {
      this.showModal = false
      this.selectedGreenHouse = null
      this.$nextTick(() => {
        this.$bvModal.hide('green-house-edit-modal')
      })
    },
    /**
     * Üvegház létrehozás vagy módosítás
     */
    updateOrCreateGreenHouse() {
      if (this.selectedGreenHouse._id){
        Axios.put('/greenHouses', this.selectedGreenHouse)
          .then(({ data }) => {
            this.$noty.success(data.status)
          })
          .catch((error) => {
            this.$noty.error(error.response.data.error)
          })
          .finally(() => {
            this.initGreenHouseList()
            this.closeModal()
          })
      } else {
        Axios.post('/greenHouses', this.selectedGreenHouse)
          .then(({ data }) => {
            this.$noty.success(data.status)
          })
          .catch((error) => {
            this.$noty.error(error.response.data.error)
          })
          .finally(() => {
            this.initGreenHouseList()
            this.closeModal()
          })
      }
    },
    /**
     * Sor törlése
     *
     * @param greenHouseData
     */
    deleteRow(greenHouseData) {
      Axios.delete('/greenHouses', {data: { id: greenHouseData._id }})
        .then(({ data }) => {
          this.$noty.success(data.status)
          this.initGreenHouseList()
        })
        .catch((error) => {
          this.$noty.error(error.response.data.error)
        })
    },
  },
}
</script>

<style lang="scss" src="./GreenHouseManagment.scss" scoped />
