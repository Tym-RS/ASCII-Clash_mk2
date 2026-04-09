// ── utils.js — reusable UI helpers ───────────────────────────

/**
 * Shake an element (e.g. on error).
 * @param {HTMLElement} el
 */
export function shake(el) {
    el.classList.remove('shake');
    void el.offsetWidth;
    el.classList.add('shake');
    el.addEventListener('animationend', () => el.classList.remove('shake'), { once: true });
}

/**
 * Show an error message in a .msg-error element.
 * @param {HTMLElement} box
 * @param {string}      msg
 */
export function showError(box, msg) {
    box.textContent = '> ' + msg;
    box.style.display = 'block';
}

/**
 * Hide a message box.
 * @param {HTMLElement} box
 */
export function hideMsg(box) {
    box.style.display = 'none';
    box.textContent = '';
}

/**
 * POST form data to a URL, returns the Response.
 * @param {string} url
 * @param {Object} fields
 * @returns {Promise<Response>}
 */
export async function postForm(url, fields) {
    const body = Object.entries(fields)
        .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
        .join('&');
    return fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body,
    });
}