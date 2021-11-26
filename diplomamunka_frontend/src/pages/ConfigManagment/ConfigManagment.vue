<template>
  <div class="container-xl text-center">
    <h3 class="my-5">Eszközök konfigurálása</h3>
    <div>
      <div v-if="deviceList.length === 0" v-text="'Nincs felvett eszköz.'" />
      <div
        v-else
        v-for="device in deviceList"
        :key="device._id"
        class="m-4 d-flex justify-content-between align-items-center device-data-wrapper"
      >
        <div @click="openModal(device)" class="d-flex justify-content-between inside-data-wrapper w-100">
          <div>
            <span class="font-weight-bold" v-text="'Id: '" />
            <span v-text="device._id"/>
          </div>
          <div>
            <span class="font-weight-bold" v-text="'deviceId: '" />
            <span v-text="device.deviceId"/>
          </div>
        </div>
      </div>
    </div>
    <b-modal
      v-if="showModal"
      id="device-config-modal"
      centered
      @hide="closeModal"
      :title="'Eszköz sorszám: ' + selectedDevice.deviceId"
    >
      <b-form-group label="Mintavételezési idő (perc)">
        <v-select
          v-model="selectedDeviceData.samplingTime"
          :options="selectOptions"
        />
      </b-form-group>
      <template #modal-footer>
        <b-btn variant="success" @click="updateConfig" v-text="'Mentés'"/>
      </template>
    </b-modal>
  </div>
</template>

<script>
import Axios from 'axios'

export default {
  name: 'ConfigManagment',
  data() {
    return {
      /**
       * Eszközök listája
       */
      deviceList: [],
      /**
       * Modal nyitott-e
       */
      showModal: false,
      /**
       * Kiválaszott eszköz
       */
      selectedDevice: null,
      /**
       * Kiválasztott eszköz konfigurációs adatai
       */
      selectedDeviceData: null,
      /**
       * Választható mintavételi idők
       */
      selectOptions: [
        5,
        10,
        20,
        30,
      ]
    }
  },
  created() {
    this.initDeviceList()
  },
  methods: {
    /**
     * Eszközök lekérése
     */
    initDeviceList() {
      Axios.get('/device')
        .then(({ data }) => {
          this.deviceList = data
        })
      .catch((error) => {
        this.$noty.error(error.response.data.error)
      })
    },
    /**
     * Modal megnyitása
     * @param device
     */
    openModal(device) {
      Axios.get('/config', {
        params: {
          deviceId: device.deviceId,
        }
      })
        .then(({ data }) => {
          this.selectedDevice = device
          this.showModal = true
          this.selectedDeviceData = data
          this.$nextTick(function () {
            this.$bvModal.show('device-config-modal')
          })
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
      this.selectedDevice = null
      this.selectedDeviceData = null
      this.$nextTick(() => {
        this.$bvModal.hide('device-config-modal')
      })
    },
    /**
     * Konfigurácios adatok frissítése
     */
    updateConfig() {
      if(this.selectedDeviceData.samplingTime === null) {
        this.$noty.error('Hibás Adat')
        return
      }

      Axios.post('/config', this.selectedDeviceData)
      .then(({ data }) => {
        this.$noty.success(data.status)
        this.closeModal()
      })
      .catch((error) => {
        this.$noty.error(error.response.data.error)
      })
    },
  },
}
</script>

<style lang="scss" src="./ConfigManagment.scss" scoped />
