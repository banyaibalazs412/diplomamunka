<template>
  <div class="container-xl py-5">
    <div v-text="'Üzenőfal'" class="title-text d-flex justify-content-center" />
    <div class="d-flex justify-content-end">
      <b-button v-text="'Új üzenőfal hozzászólás'" @click="openModal" />
    </div>
    <div v-if="noteBoardItems.length > 0">
      <div v-for="noteBoardItem in noteBoardItems" :key="noteBoardItem._id" class="p-5">
        <div class="forum-item-wrapper">
          <div
            class="forum-title"
            v-text="noteBoardItem.title + ' (' + noteBoardItem.name + ' - ' +
              moment(String(noteBoardItem.date)).format('YYYY-MM-DD hh:mm:ss')  + ')'"
          />
          <hr>
          <div class="forum-description" v-text="noteBoardItem.description" />
        </div>
      </div>
    </div>
    <div v-else>

    </div>
    <b-modal
      v-if="showModal"
      id="note-board-modal"
      size="xl"
      centered
      @hide="closeModal"
      :title="'Üzenőfal hozzászólás készítése'"
      class="modal-wrapper"
    >
     <b-form-group label="Fejléc:">
       <b-form-input
         v-model="newNoteBoardItem.title"
       />
     </b-form-group>
      <b-form-group label="Szöveg:">
        <b-form-textarea
          v-model="newNoteBoardItem.description"
        />
      </b-form-group>
      <template #modal-footer>
        <div class="d-flex justify-content-end">
          <b-button
            v-text="'Mentés'"
            :disabled="newNoteBoardItem.title === '' || newNoteBoardItem.description === ''"
            @click="saveNoteBoardComment"
          />
        </div>
      </template>
    </b-modal>
  </div>
</template>

<script>
import Axios from 'axios'
import moment from 'moment'

export default {
  name: 'Forum',
  data () {
    return {
      /**
       * moment példány
       */
      moment: moment,
      /**
       * Új üzenőfal comment
       */
      newNoteBoardItem: {
        title: '',
        description: '',
      },
      /**
       * Látható-e a modal
       */
      showModal: false,
      /**
       * Üzenőfal commentek
       */
      noteBoardItems: [],
    }
  },
  created() {
    this.initNoteBoard()
  },
  methods: {
    /**
     * Üzenőfal commentek betöltése
     */
    initNoteBoard() {
      Axios.get('/noteBoard')
      .then(({ data }) => {
        this.noteBoardItems = data
      })
      .catch((error) => {
        this.$noty.error(error.response.data.error)
      })
    },
    /**
     * Modal megnyitása
     */
    openModal() {
      this.showModal = true
      this.$nextTick(function () {
        this.$bvModal.show('note-board-modal')
      })
    },
    /**
     * Modal bezárása adatok ürítése
     */
    closeModal() {
      this.showModal = false
      this.newNoteBoardItem.title = ''
      this.newNoteBoardItem.description = ''
      this.$nextTick(() => {
        this.$bvModal.hide('note-board-modal')
      })
    },
    /**
     * Üzenőfal comment mentése
     */
    saveNoteBoardComment() {
      Axios.post('/noteBoard', this.newNoteBoardItem)
        .then(({ data }) => {
          this.$noty.success(data.status)
          this.closeModal()
        })
        .catch((error) => {
          console.error(error)
          this.$noty.error(error.response.data.error)
        })
        .finally(() => {
          this.initNoteBoard()
        })
    }
  },

}
</script>

<style lang="scss" src="./NoteBoard.scss" scoped />
