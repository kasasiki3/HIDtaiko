(function () {
  // プライベートウィンドウやサイトデータ拒否の設定では localStorage 自体が例外を投げる
  let lang = 'ja';
  try { lang = localStorage.getItem('hidtaiko-lang') || 'ja'; } catch (e) {}
  apply(lang);

  const cb = document.getElementById('langToggle');
  if (!cb) return;
  cb.checked = (lang === 'en');

  cb.addEventListener('change', () => {
    const next = cb.checked ? 'en' : 'ja';
    apply(next);
    try { localStorage.setItem('hidtaiko-lang', next); } catch (e) {}
  });

  // body のクラスだけでなく html の lang も変える。合わないと読み上げ音声が日本語のままになる
  function apply(value) {
    document.body.classList.remove('is-ja', 'is-en');
    document.body.classList.add('is-' + value);
    document.documentElement.lang = value;
  }
})();
