(function () {
  const lang = localStorage.getItem('hidtaiko-lang') || 'ja';
  document.body.classList.add('is-' + lang);

  const cb = document.getElementById('langToggle');
  if (!cb) return;
  cb.checked = (lang === 'en');

  cb.addEventListener('change', () => {
    const next = cb.checked ? 'en' : 'ja';
    document.body.classList.remove('is-ja', 'is-en');
    document.body.classList.add('is-' + next);
    localStorage.setItem('hidtaiko-lang', next);
  });
})();
