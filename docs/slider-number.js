// スライダー横の数値欄（<input class="slider-num">）をキーボードで直接編集できるようにする。
// v11 / v12 / v13 の各ページで共通に使う。
//
// 1文字ごとに反映すると「120」を打つ途中の「1」でスライダーが跳ねるので、
// Enter かフォーカスを外したときにだけ確定する。送信ボタンを押せばフォーカスが外れて
// 先に確定するので、打ってすぐ送信しても値は乗る。
// 確定値は整数へ丸めてスライダーの「今の」min〜max に収める（初心者/高度な設定で範囲が変わる）。
// step には丸めない。5刻みのスライダーでは選べない値を打てるようにするのが目的なので、
// つまみは近い目盛りへ吸着し、数値欄には打った値（＝送信する値）を出す。
(function () {
  // IMEがオンのままでも打てるよう、全角数字・全角マイナス・全角ピリオドを半角へ寄せる
  function parse(text) {
    const s = String(text)
      .replace(/[０-９]/g, c => String.fromCharCode(c.charCodeAt(0) - 0xFEE0))
      .replace(/[－−]/g, "-")
      .replace(/．/g, ".")
      .trim();
    if (!/^-?\d+(\.\d+)?$/.test(s)) return NaN;
    return Math.round(Number(s));
  }

  // apply(v) には確定した値が渡る。kando の更新や連動するスライダーの同期はページ側で行う
  window.bindSliderNumber = function (slider, num, apply) {
    if (!slider || !num) return;
    let last = num.value;        // Esc や不正な入力のときに戻す値
    let keepSelection = false;   // クリックで入ったときの全選択を mouseup で崩さない

    function commit(v) {
      v = Math.min(+slider.max, Math.max(+slider.min, v));
      slider.value = v;
      num.value = v;
      last = num.value;
      apply(v);
    }

    // 打ち込んだ文字列を確定する。手を付けていなければ何もしない
    // （受信した範囲外の値を、触っただけでクランプしてしまわないように）
    function commitText() {
      if (num.value === last) return;
      const v = parse(num.value);
      if (Number.isNaN(v)) { num.value = last; return; }
      commit(v);
    }

    num.addEventListener("focus", () => {
      last = num.value;
      num.select();
      keepSelection = true;
    });
    num.addEventListener("mouseup", e => {
      if (keepSelection) e.preventDefault();
      keepSelection = false;
    });
    num.addEventListener("blur", () => { keepSelection = false; });
    num.addEventListener("change", commitText);

    num.addEventListener("keydown", e => {
      // 変換確定のEnterは値の確定ではない
      if (e.isComposing || e.keyCode === 229) return;
      if (e.key === "Enter") {
        e.preventDefault();
        commitText();
        num.select();
      } else if (e.key === "Escape") {
        num.value = last;
        num.select();
      } else if (e.key === "ArrowUp" || e.key === "ArrowDown") {
        // キーボードだけで微調整できるよう ±1（Shift で ±10）
        e.preventDefault();
        let base = parse(num.value);
        if (Number.isNaN(base)) base = parse(last);
        if (Number.isNaN(base)) base = +slider.value;
        commit(base + (e.key === "ArrowUp" ? 1 : -1) * (e.shiftKey ? 10 : 1));
        num.select();
      }
    });
  };
})();
