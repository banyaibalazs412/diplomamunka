import Vue from 'vue'
import VueRouter from 'vue-router'
import Home from '../views/Home.vue'
import Login from '../pages/Login/Login.vue'
import Register from '../pages/Register/Register.vue'
import UserManagment from '../pages/UserManagment/UserManagment.vue'
import GreenHouseManagment from '../pages/GreenHouseManagment/GreenHouseManagment.vue'
import DeviceManagment from '../pages/DeviceManagment/DeviceManagment.vue'
import DeviceData from '../pages/DeviceData/DeviceData.vue'
import NoteBoard from '../pages/NoteBoard/NoteBoard.vue'
import ConfigManagment from '../pages/ConfigManagment/ConfigManagment.vue'
import { roles } from '../constans/RolesConstant'
import NotFound from "../components/NotFound/NotFound";

Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    name: 'Home',
    component: Home,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_USER, roles.ROLE_ADMIN, roles.ROLE_SUPERADMIN],
    },
  },
  {
    path: '/login',
    name: 'Login',
    component: Login,
    meta: {
      requiresAuth: false,
    },
  },
  {
    path: '/register',
    name: 'Register',
    component: Register,
    meta: {
      requiresAuth: false,
    },
  },
  {
    path: '/user-managment',
    name: 'UserManagment',
    component: UserManagment,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_ADMIN, roles.ROLE_SUPERADMIN],
    },
  },
  {
    path: '/green-house-managment',
    name: 'GreenHouseManagment',
    component: GreenHouseManagment,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_SUPERADMIN],
    },
  },
  {
    path: '/device-managment',
    name: 'DeviceManagment',
    component: DeviceManagment,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_ADMIN, roles.ROLE_SUPERADMIN],
    },
  },
  {
    path: '/device-data',
    name: 'DeviceData',
    component: DeviceData,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_USER, roles.ROLE_ADMIN],
    },
  },
  {
    path: '/note-board',
    name: 'NoteBoard',
    component: NoteBoard,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_USER, roles.ROLE_ADMIN],
    },
  },
  {
    path: '/config-managment',
    name: 'ConfigManagment',
    component: ConfigManagment,
    meta: {
      requiresAuth: true,
      canReach: [roles.ROLE_ADMIN],
    },
  },
  {
    path: '/*',
    name: 'NotFound',
    component: NotFound,
    meta: {
      requiresAuth: false,
    },
  }
]

const router = new VueRouter({
  mode: 'history',
  base: process.env.BASE_URL,
  routes
})

/**
 * Mielőtt belépünk az adott route-ra meg kell vizsgálni, hogy van e hozzá jogosultsága
 * Ha nincs belépve senki, de szükséges az adott oldalhoz, akkor át irányítjuk a login pagere
 */
router.beforeEach((to, from, next) => {
  const role = localStorage.getItem('role')
  const token = localStorage.getItem('token')
  const greenHouseId = localStorage.getItem('greenHouseId')

  if (to.path === '/login' && role && token) {
    return router.push('/')
  }

  if (!to.meta.requiresAuth) {
   return next();
  }

  if (role && token ) {
    if (to.meta.canReach.includes(role)) {
      if (to.meta.canReach.includes(roles.ROLE_ADMIN) || (greenHouseId !== null)) {
        return next();
      }
    }
  }

  router.push('/login')
})

export default router
