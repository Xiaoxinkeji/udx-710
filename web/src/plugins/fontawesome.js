/**
 * Font Awesome 按需引入配置
 * 只引入项目实际使用的图标，大幅减少打包体积
 * 使用 DOM 监视器自动转换 <i> 标签为 SVG
 */
import { library, dom } from '@fortawesome/fontawesome-svg-core'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'

// 导入 Brands 图标 (fab 前缀)
import { faGithub, faUsb } from '@fortawesome/free-brands-svg-icons'

// 导入项目使用的所有图标
import {
  // App.vue 菜单图标
  faTachometerAlt,
  faNetworkWired,
  faWifi,
  faTowerCell,
  faEnvelope,
  faChartArea,
  faBatteryHalf,
  faTerminal,
  faSlidersH,
  faChevronLeft,
  faChevronRight,
  faBars,
  faSyncAlt,

  // SystemMonitor.vue
  faSatelliteDish,
  faBroadcastTower,
  faWaveSquare,
  faHashtag,
  faSignal,
  faChartLine,
  faServer,
  faMicrochip,
  faClock,
  faTemperatureHalf,
  faMemory,
  faBroom,
  faFingerprint,
  faSimCard,
  faBuilding,
  faGaugeHigh,
  faGauge,
  faArrowDown,
  faArrowUp,
  faHeartbeat,
  faTemperatureHigh,
  faPlug,
  faHeartPulse,
  faBatteryThreeQuarters,
  faInfoCircle,
  faDesktop,
  faCodeBranch,

  // NetworkManager.vue
  faPlane,
  faCheckCircle,

  // WifiManager.vue
  faCog,
  faPowerOff,
  faBolt,
  faLayerGroup,
  faSignature,
  faKey,
  faSliders,
  faShieldHalved,
  faDice,
  faSave,
  faTowerBroadcast,
  faShield,
  faCircleInfo,

  // TrafficStats.vue
  faChartPie,
  faArrowTrendUp,
  faToggleOn,
  faTrash,
  faLightbulb,

  // BatteryManager.vue
  faHeart,
  faBalanceScale,
  faThumbsUp,
  faExclamation,
  faTriangleExclamation,
  faPlay,
  faStop,
  faFire,
  faSkull,
  faSnowflake,
  faQuestion,

  // AdvancedNetwork.vue
  faLock,
  faUnlock,
  faCheck,
  faTimes,
  faSpinner,
  faStar,
  faExclamationTriangle,

  // ATDebug.vue
  faPaperPlane,
  faCode,
  faHistory,

  // SystemSettings.vue
  faCalendarAlt,
  faSignOutAlt,
  faSignInAlt,

  // 主题切换图标
  faSun,
  faMoon,

  // DeviceManager.vue
  faLaptop,
  faBan,
  faShieldAlt,
  faUndo,
  faInbox,
  faChevronUp,
  faChevronDown,
  faExclamationCircle,

  // WifiManager.vue 补充
  faCopy,
  faEye,
  faEyeSlash,

  // SmsManager.vue 补充
  faShareNodes,
  faGear,
  faUser,
  faBell,

  // SystemUpdate.vue
  faCloudDownloadAlt,
  faCloudUploadAlt,
  faRocket,
  faFileArchive,
  faLink,

  // SystemSettings.vue 恢复出厂设置
  faRotateLeft,

  // GlobalConfirm.vue
  faQuestionCircle,

  // SmsManager.vue 配置页面
  faWrench,

  // WebTerminal.vue
  faExternalLinkAlt,
  faExpand,
  faCompress,

  // NetworkManager.vue 漫游图标
  faGlobe,

  // PluginStore.vue 插件商城
  faPuzzlePiece,
  faPlus,
  faDownload,
  faUpload,
  faTrashAlt,
  faFileCode,
  faFileExport,
  faFileImport,
  faBook,
  faEdit,
  faToolbox,
  faFolder,
  faDatabase,
  faSearch,

  // ApnConfig.vue 运营商图标
  faMobileAlt,

  // AutomationManager.vue 自动化流图标
  faRobot
} from '@fortawesome/free-solid-svg-icons'

// 注册所有图标到库
library.add(
  faTachometerAlt,
  faNetworkWired,
  faWifi,
  faTowerCell,
  faEnvelope,
  faChartArea,
  faBatteryHalf,
  faTerminal,
  faSlidersH,
  faChevronLeft,
  faChevronRight,
  faBars,
  faSyncAlt,
  faSatelliteDish,
  faBroadcastTower,
  faWaveSquare,
  faHashtag,
  faSignal,
  faChartLine,
  faServer,
  faMicrochip,
  faClock,
  faTemperatureHalf,
  faMemory,
  faBroom,
  faFingerprint,
  faSimCard,
  faBuilding,
  faGaugeHigh,
  faGauge,
  faArrowDown,
  faArrowUp,
  faHeartbeat,
  faTemperatureHigh,
  faPlug,
  faHeartPulse,
  faBatteryThreeQuarters,
  faInfoCircle,
  faDesktop,
  faCodeBranch,
  faGithub,
  faPlane,
  faCheckCircle,
  faCog,
  faPowerOff,
  faBolt,
  faLayerGroup,
  faSignature,
  faKey,
  faSliders,
  faShieldHalved,
  faDice,
  faSave,
  faTowerBroadcast,
  faShield,
  faCircleInfo,
  faChartPie,
  faArrowTrendUp,
  faToggleOn,
  faTrash,
  faLightbulb,
  faHeart,
  faBalanceScale,
  faThumbsUp,
  faExclamation,
  faTriangleExclamation,
  faPlay,
  faStop,
  faFire,
  faSkull,
  faSnowflake,
  faQuestion,
  faLock,
  faUnlock,
  faCheck,
  faTimes,
  faSpinner,
  faStar,
  faExclamationTriangle,
  faPaperPlane,
  faCode,
  faHistory,
  faCalendarAlt,
  faSignOutAlt,
  faSignInAlt,
  faSun,
  faMoon,
  faLaptop,
  faBan,
  faShieldAlt,
  faUndo,
  faInbox,
  faChevronUp,
  faChevronDown,
  faExclamationCircle,
  faCopy,
  faEye,
  faEyeSlash,
  faShareNodes,
  faGear,
  faUser,
  faBell,
  faCloudDownloadAlt,
  faCloudUploadAlt,
  faRocket,
  faFileArchive,
  faLink,
  faRotateLeft,
  faQuestionCircle,
  faWrench,
  faExternalLinkAlt,
  faExpand,
  faCompress,
  faUsb,
  faGlobe,
  faPuzzlePiece,
  faPlus,
  faDownload,
  faUpload,
  faTrashAlt,
  faFileCode,
  faFileExport,
  faFileImport,
  faBook,
  faEdit,
  faToolbox,
  faFolder,
  faDatabase,
  faSearch,
  faMobileAlt,
  faRobot
)

// 启用 DOM 监视器，自动将 <i class="fas fa-xxx"> 转换为 SVG
dom.watch()

export { FontAwesomeIcon }
