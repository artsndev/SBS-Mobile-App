import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  {
    path: '/',
    redirect: {
      name: 'App'
    }
  },
  {
    path: '/app',
    name: 'App',
    component: () => import('@/components/App.vue')
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})
export default router
