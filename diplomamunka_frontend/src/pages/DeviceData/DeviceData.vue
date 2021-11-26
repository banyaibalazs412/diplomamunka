<template>
  <div class="container-xl py-5">
    <div v-text="'Eszköz adatok Lekérdezése'" class="title-text d-flex justify-content-center" />
    <div class="row">
      <div class="col-12 col-lg-6">
        <div
          v-if="selectedColumn === null || selectedRow === null"
          v-text="'Válasszon oszlopot és sort.'"
          class="hint-text"
        />
        <b-form-group  label="Oszlop:">
          <v-select
            :options="getColumns"
            v-model="selectedColumn"
            :searchable="false"
            @input="selectValueChange"
          />
        </b-form-group>
        <b-form-group  label="Sor:">
          <v-select
            :options="getRows"
            v-model="selectedRow"
            :searchable="false"
            @input="selectValueChange"
          />
        </b-form-group>
        <div v-text="'Üvegházhoz rendelt eszközök száma: ' + this.deviceNumber" />
      </div>
      <div class="col-12 col-lg-6 device-wrapper">
        <div v-if="deviceList.length > 0">
          <div
            v-for="device in deviceList"
            :key="device._id"
            class="m-4 d-flex justify-content-between align-items-center device-data-wrapper"
          >
            <div @click="openDeviceModal(device)" class="d-flex justify-content-between inside-data-wrapper w-100">
              <div>
                <span class="font-weight-bold" v-text="'Id: '" />
                <span v-text="device._id"/>
              </div>
              <div>
                <span class="font-weight-bold" v-text="'deviceId: '" />
                <span v-text="device.deviceId"/>
              </div>
            </div>
              <span
                v-if="role === roles.ROLE_ADMIN"
                v-b-tooltip="'Azonnali lekérdezés indítása'"
                class="icon-wrapper"
                :class="{ 'disabled-refresh': !isSendDataButtonEnabled }"
                @click="startMeasurement(device)"
              >
                <font-awesome-icon icon="sync"/>
              </span>
          </div>
        </div>
        <div
          v-else-if="selectedColumn !== null && selectedRow !== null"
          class="d-flex justify-content-center"
          v-text="'Nincs az adott oszlopnak és sornak megfelelő eszköz!'"
        />
        <div
          v-else
          class="d-flex justify-content-center"
          v-text="'Adja meg az oszlopot és a sort.'"
        />
      </div>
    </div>
    <b-modal
      v-if="showModal"
      id="device-data-modal"
      size="xl"
      centered
      hide-footer
      @hide="closeModal"
      :title="'Eszköz sorszám: ' + selectedDevice.deviceId"
      class="modal-wrapper"
    >
      <b-form-datepicker
        v-model="selectedDate"
        placeholder="Válasszon dátumot!"
        class="mb-2"
        @input="getDeviceData()"
        reset-button
        label-reset-button="Törlés"
      />
      <div class="user-managment-wrapper">
        <b-table
          sort-icon-left
          striped
          show-empty
          :items="selectedDeviceDataList"
          :fields="fields"
        >
          <template #empty>
            <div class="text-center" v-text="'Nincs elem a listában'" />
          </template>
          <template  #cell(date)="data">
            <div v-text="moment(String(data.item.date)).format('YYYY-MM-DD hh:mm:ss')" />
          </template>
        </b-table>
      </div>
    </b-modal>
  </div>
</template>

<script>
import Axios from 'axios'
import moment from 'moment'
import { mapGetters } from 'vuex'
import { library } from '@fortawesome/fontawesome-svg-core'
import { faSync } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'
import { roles } from '../../constans/RolesConstant.js'

library.add(faSync)

export default {
  name: 'DeviceData',
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
       * moment példány
       */
      moment: moment,
      /**
       * Maximum oszlop
       */
      maxColumn: 0,
      /**
       * Maximum sor
       */
      maxRow: 0,
      /**
       * Eszközök száma
       */
      deviceNumber: 0,
      /**
       * Kiválaszott oszlop
       */
      selectedColumn: null,
      /**
       * Kiválasztott sor
       */
      selectedRow: null,
      /**
       * Helyes-e az érték
       */
      isValueValid: false,
      /**
       * Eszközök listája
       */
      deviceList: [],
      /**
       * Modal látható-e
       */
      showModal: false,
      /**
       * Kiválasztott eszköz
       */
      selectedDevice: null,
      /**
       * Kiválaszott eszköz adatai
       */
      selectedDeviceDataList: [],
      /**
       * Mérési kérés
       */
      deviceRequest: null,
      /**
       * Kiválasztott dátum
       */
      selectedDate: null,
      /**
       * Azonnali frissítés gomb inaktiválásáért felel
       */
      isSendDataButtonEnabled: true,
      /**
       * Táblázat mezők
       */
      fields: [
        {
          key: 'temperature',
          label: 'Hőmérséklet(°C)',
          sortable: false
        },
        {
          key: 'humidity',
          label: 'Páratartalom(%)',
          sortable: true
        },
        {
          key: 'laserDistance',
          label: 'Távolság(Lézer [mm])',
          sortable: true
        },
        {
          key: 'sonicDistance',
          label: 'Távolság(UH [mm])',
          sortable: true
        },
        {
          key: 'weight',
          label: 'Tömeg(g)',
          sortable: true
        },
        {
          key: 'moisture',
          label: 'Föld nedvesség',
          sortable: true
        },
        {
          key: 'colorRed',
          label: 'Szín (Piros)',
          sortable: false
        },
        {
          key: 'colorGreen',
          label: 'Szín (Zöld)',
          sortable: false
        },
        {
          key: 'colorBlue',
          label: 'Szín (Kék)',
          sortable: false
        },
        {
          key: 'date',
          label: 'Dátum',
          sortable: false
        },
      ],
    }
  },
  computed: {
    ...mapGetters({
      role: 'getRole',
    }),
    /**
     * Vissza adja az oszlopokat a max oszlop szám alapján
     *
     * @return {*[]}
     */
    getColumns() {
      const columnOptions = []
      for(let i = 1; i <= this.maxColumn; i++) {
        columnOptions.push(i)
      }
      return columnOptions;
    },
    /**
     * Vissza adja az sorokat a max sor szám alapján
     *
     * @return {*[]}
     */
    getRows() {
      const rowOptions = []
      for(let i = 1; i <= this.maxRow; i++) {
        rowOptions.push(i)
      }
      return rowOptions;
    },
  },
  created() {
    this.getMaxColAndRow()
  },
  methods: {
    /**
     * Lekérdezi a maximum oszlop és sor számot
     */
    getMaxColAndRow() {
      Axios.get('/greenHouses/maxColAndRow')
        .then(({ data }) => {
          this.maxColumn = data.maxColumn
          this.maxRow = data.maxRow
          this.deviceNumber = data.deviceNumber
        })
        .catch((error) => {
          console.error(error)
        })
    },
    /**
     * Kiválasztott érték változására bekövetkező event
     */
    selectValueChange() {
      this.deviceList = []
      if (this.selectedColumn !== null && this.selectedRow !== null) {
        this.isValueValid = true
      } else {
        this.isValueValid = false
      }

      if (this.isValueValid) {
        Axios.get('/device/deviceList',
          {params:
            {
              column: this.selectedColumn,
              row: this.selectedRow
            }
        })
          .then(({ data }) => {
            this.deviceList = data
          })
          .catch((error) => {
            console.error(error)
          })
      }
    },
    /**
     * Modal megnyitása
     *
     * @param device
     */
    openDeviceModal(device) {
      this.showModal = true
      this.selectedDevice = device
      this.$nextTick(function () {
        this.$bvModal.show('device-data-modal')
      })
      this.getDeviceData()
    },
    /**
     * Eszköz adatok lekérése
     */
    getDeviceData() {
      Axios.get('/device/deviceData',
        {params:
            {
              _id: this.selectedDevice._id,
              date: this.selectedDate,
            }
        })
      .then(({ data }) => {
        this.selectedDeviceDataList = data
      })
      .catch((error) => {
        this.$noty.error(error.response.data.error)
      })
    },
    /**
     * Modal bezárása
     */
    closeModal() {
      this.showModal = false
      this.selectedDeviceDataList = []
      this.selectedDate = null
      this.$nextTick(() => {
        this.$bvModal.hide('device-data-modal')
      })
    },
    /**
     * Azonnali mérés igény küldése
     *
     * @param device
     */
    startMeasurement(device) {
      if (!this.isSendDataButtonEnabled) {
        return
      }

      this.isSendDataButtonEnabled = false
      Axios.get('/device/startMeasurement', {
        params: {
          deviceId: device.deviceId,
        }
      })
        .then(({ data }) => {
          this.$noty.success(data.status)
        })
        .catch((error) => {
          this.$noty.error(error.response.data.error)
        })
        .finally(() => {
          setTimeout(() => {
            this.isSendDataButtonEnabled = true
          }, 2000)
        })
    }
  }
}
</script>

<style lang="scss" src="./DeviceData.scss" scoped />
