// utils.js — global helpers for ASCII Clash (classic script, no ES modules)

async function apiFetch(url, options) {
  try {
    var res = await fetch(url, options || {});
    if (res.status === 308) { window.location.href = '/index.html'; return null; }
    return res;
  } catch (e) {
    console.error('[apiFetch]', e);
    return null;
  }
}

function shake(el) {
  el.classList.remove('shake');
  void el.offsetWidth;
  el.classList.add('shake');
  el.addEventListener('animationend', function() { el.classList.remove('shake'); }, { once: true });
}

function showError(box, msg) {
  box.textContent = '> ' + msg;
  box.style.display = 'block';
}

function hideMsg(box) {
  box.textContent = '';
  box.style.display = 'none';
}

async function postForm(url, fields) {
  return apiFetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams(fields).toString()
  });
}

async function postJSON(url, data) {
  return apiFetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  });
}

async function post(url) {
  return apiFetch(url, { method: 'POST' });
}
