/**
 * @file logger.js
 * @brief 生产环境日志过滤工具
 */

const isDev = process.env.NODE_ENV === 'development' || window.location.hostname === 'localhost';

export const logger = {
    log: (...args) => {
        if (isDev) console.log('[LOG]', ...args);
    },
    info: (...args) => {
        if (isDev) console.info('[INFO]', ...args);
    },
    warn: (...args) => {
        if (isDev) console.warn('[WARN]', ...args);
    },
    error: (...args) => {
        // 错误日志在生产环境也保留，但可以增加前缀便于区分
        console.error('[CRITICAL]', ...args);
    },
    debug: (...args) => {
        if (isDev) console.debug('[DEBUG]', ...args);
    }
};

export default logger;
