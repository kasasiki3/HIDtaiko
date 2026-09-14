// GA4へ計測イベントを送る。gtagが未ロード・広告ブロックで無効でも、呼び出し側の処理を壊さない
(function (global) {
  function track(name, params) {
    if (typeof global.gtag !== "function") return;
    try { global.gtag("event", name, params || {}); } catch (e) {}
  }

  // 外部リンクのうちGitHubだけは流入の質を見たいので専用イベントにする
  document.addEventListener("click", function (event) {
    var a = event.target && event.target.closest && event.target.closest("a[href]");
    if (!a) return;
    if (!/^https?:\/\/(www\.)?github\.com\//i.test(a.href)) return;
    track("outbound_github", { link_url: a.href });
  });

  global.GaEvents = { track: track };
})(window);
