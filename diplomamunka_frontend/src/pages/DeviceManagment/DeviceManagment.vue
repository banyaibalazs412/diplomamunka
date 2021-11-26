<template>
  <div class="container-xl">
    <h2 class="text-center my-5" v-text="'Eszközök módosítása'" />
    <div class="w-100 text-right pb-5">
      <b-button v-text="'Új eszköz létrehozása'" @click="openModal" />
    </div>
    <div class="device-managment-wrapper">
      <b-table
        sort-icon-left
        striped
        show-empty
        :busy="!isDeviceListLoaded"
        :sort-by.sync="sortBy"
        :sort-desc.sync="sortDesc"
        :items="deviceList"
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
          <span class="icon-wrapper" @click="deleteRow(data.item)">
            <font-awesome-icon icon="trash-alt"/>
          </span>
        </template>
      </b-table>
    </div>
    <b-modal
      v-if="showModal"
      id="device-edit-modal"
      centered
      @hide="closeModal"
      :title="'Új eszköz létrehozása'"
    >
      <b-form-group label="DeviceId:">
        <b-form-input v-model="newDevice.deviceId" type="number" />
      </b-form-group>
      <b-form-group v-if="getRole === roles.ROLE_SUPERADMIN" label="Üvegház:">
        <b-form-input v-model="newDevice.greenHouseId" />
      </b-form-group>
      <b-form-group label="Ip cím:">
        <b-form-input v-model="newDevice.ipAddress" />
      </b-form-group>
      <b-form-group label="Oszlop:">
        <b-form-input v-model="newDevice.column" type="number" min="1" :max="this.maxColumn" />
      </b-form-group>
      <b-form-group label="Sor:">
        <b-form-input v-model="newDevice.row" type="number" min="1" :max="this.maxRow" />
      </b-form-group>
      <template #modal-footer>
        <b-btn variant="success" @click="createDevice" v-text="'Mentés'"/>
      </template>
    </b-modal>
  </div>
</template>

<script>
import Axios from 'axios'
import { mapGetters } from 'vuex'
import { library } from '@fortawesome/fontawesome-svg-core'
import { faEdit, faTrashAlt } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'
import { roles } from '../../constans/RolesConstant.js'

library.add(faEdit, faTrashAlt)

export default {
  name: 'DeviceManagment',
  components: {
    FontAwesomeIcon,
  },
  data() {
    return {
      /**
       * Jogosultság constansok
       */
      roles: roles,
      /**
       * Modal megjelenik-e
       */
      showModal: false,
      /**
       * Betöltött-e az eszköz lista
       */
      isDeviceListLoaded: false,
      /**
       * Eszköz lista
       */
      deviceList: [],
      /**
       * Új eszköz objektum
       */
      newDevice: {
        deviceId: 1,
        greenHouseId: '',
        column: 1,
        row: 1,
      },
      /**
       * Lista rendezés
       */
      sortBy: '',
      /**
       * Lista rendezés
       */
      sortDesc: '',
      /**
       * Maximum oszlopok száma
       */
      maxColumn: 1,
      /**
       * Maximum sorok száma
       */
      maxRow: 1,
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
          key: 'deviceId',
          label: 'DeviceId',
          sortable: true,
        },
        {
          key: 'ipAddress',
          label: 'Ip cím',
          sortable: true,
        },
        {
          key: 'column',
          label: 'Oszlop',
          sortable: false,
        },
        {
          key: 'row',
          label: 'Sor',
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
  computed: {
    ...mapGetters({
      getRole: 'getRole',
    }),
  },
  created() {
    this.initDeviceList()
    this.getMaxColAndRow()
  },
  methods: {
    /**
     * Eszköz lista inicializálása
     */
    initDeviceList() {
      this.isDeviceListLoaded = false
      Axios.get('/device')
        .then(({ data }) => {
          this.deviceList = data
        })
        .catch( error  => {
          this.$noty.error(error.response.data.error)
        })
        .finally(() => {
          this.isDeviceListLoaded = true
        })
    },
    /**
     * Lekérdezi a maximum oszlop és sor számot
     */
    getMaxColAndRow() {
      Axios.get('/greenHouses/maxColAndRow')
          .then(({ data }) => {
            this.maxColumn = data.maxColumn
            this.maxRow = data.maxRow
          })
          .catch((error) => {
            console.error(error)
          })
    },
    /**
     * Eszköz törlése
     */
    deleteRow(device) {
      console.log(device._id)
      Axios.delete('/device', {data: { id: device._id }})
        .then(({ data }) => {
          this.$noty.success(data.status)
          this.initDeviceList()
        })
        .catch((error) => {
          this.$noty.error(error.response.data.error)
        })
      .finally(() => this.initDeviceList())
    },
    /**
     * Eszköz létrehozása
     */
    createDevice() {
      Axios.post('/device', this.newDevice)
        .then(({ data }) => {
          this.$noty.success(data.status)
        })
        .catch((error) => {
          this.$noty.error(error.response.data.error)
        })
        .finally(() => {
          this.initDeviceList()
          this.closeModal()
        })
    },
    /**
     * Modal megnyitása a létrehozáshoz
     */
    openModal() {
      this.showModal = true
      this.$nextTick(function () {
        this.$bvModal.show('device-edit-modal')
      })
    },
    /**
     * Modal bezárása és adat ürítés
     */
    closeModal() {
      this.showModal = false
      this.$nextTick(() => {
        this.$bvModal.hide('device-edit-modal')
      })
    },
  }
}
</script>

<style lang="scss" src="./DeviceManagment.scss" scoped />
