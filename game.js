import { palettes, sprites, drawSprite } from './sprites.js';

const canvas = document.getElementById('game');
const ctx = canvas.getContext('2d');

const hpEl = {
  vivi: document.getElementById('viviHp'),
  wisky: document.getElementById('wiskyHp'),
  maliketh: document.getElementById('malikethHp'),
};

const logBox = document.getElementById('log');
const ending = document.getElementById('ending');
const restart = document.getElementById('restart');

const world = {
  width: canvas.width,
  height: canvas.height,
  flowers: Array.from({ length: 70 }, () => ({ x: rand(0, canvas.width), y: rand(210, canvas.height - 12), c: rand(0, 1) })),
};

const state = {
  over: false,
  vivi: { x: 120, y: 380, hp: 100, speed: 2.6 },
  wisky: { x: 185, y: 400, hp: 100, speed: 2.9 },
  maliketh: { x: 760, y: 360, hp: 220, dir: 1 },
  attacks: [],
  keys: new Set(),
};

function rand(min, max) { return Math.floor(Math.random() * (max - min + 1)) + min; }
function clamp(v, min, max) { return Math.max(min, Math.min(max, v)); }

function log(text) {
  const p = document.createElement('p');
  p.textContent = `• ${text}`;
  logBox.prepend(p);
}

function rectHit(a, b) {
  return !(a.x + a.w < b.x || a.x > b.x + b.w || a.y + a.h < b.y || a.y > b.y + b.h);
}

function spawnAttack(owner) {
  if (state.over) return;
  if (owner === 'vivi') {
    state.attacks.push({ owner, x: state.vivi.x + 40, y: state.vivi.y + 28, vx: 5.8, w: 16, h: 10, dmg: rand(12, 20), label: 'Ráfaga de Puños Tiernos' });
    log('Vivi lanza una ráfaga de puños tiernos 💗🥊');
  } else {
    state.attacks.push({ owner, x: state.wisky.x + 30, y: state.wisky.y + 22, vx: 6.4, w: 12, h: 8, dmg: rand(10, 22), label: 'Arañazo de Wisky' });
    log('Wisky hace un arañazo ultra-latoso 🐈‍⬛⚡');
  }
}

function enemyAttack() {
  if (state.over) return;
  const target = Math.random() < 0.5 ? state.vivi : state.wisky;
  const damage = rand(6, 13);
  target.hp -= damage;
  log(`Maliketh invoca nube de pelusa y golpea por ${damage}.`);
}

function update() {
  if (state.over) return;

  if (state.keys.has('w')) state.vivi.y -= state.vivi.speed;
  if (state.keys.has('s')) state.vivi.y += state.vivi.speed;
  if (state.keys.has('a')) state.vivi.x -= state.vivi.speed;
  if (state.keys.has('d')) state.vivi.x += state.vivi.speed;

  if (state.keys.has('arrowup')) state.wisky.y -= state.wisky.speed;
  if (state.keys.has('arrowdown')) state.wisky.y += state.wisky.speed;
  if (state.keys.has('arrowleft')) state.wisky.x -= state.wisky.speed;
  if (state.keys.has('arrowright')) state.wisky.x += state.wisky.speed;

  state.vivi.x = clamp(state.vivi.x, 0, world.width - 64);
  state.vivi.y = clamp(state.vivi.y, 180, world.height - 64);
  state.wisky.x = clamp(state.wisky.x, 0, world.width - 64);
  state.wisky.y = clamp(state.wisky.y, 180, world.height - 64);

  state.maliketh.x += state.maliketh.dir * 1.2;
  if (state.maliketh.x < 610 || state.maliketh.x > 860) state.maliketh.dir *= -1;

  state.attacks.forEach((a) => { a.x += a.vx; });
  state.attacks = state.attacks.filter((a) => a.x < world.width + 20);

  const enemyRect = { x: state.maliketh.x, y: state.maliketh.y, w: 64, h: 64 };
  state.attacks = state.attacks.filter((a) => {
    const attackRect = { x: a.x, y: a.y, w: a.w, h: a.h };
    if (rectHit(attackRect, enemyRect)) {
      state.maliketh.hp -= a.dmg;
      log(`${a.label} conecta y hace ${a.dmg} daño.`);
      if (Math.random() < 0.35) enemyAttack();
      return false;
    }
    return true;
  });

  hpEl.vivi.textContent = Math.max(0, Math.floor(state.vivi.hp));
  hpEl.wisky.textContent = Math.max(0, Math.floor(state.wisky.hp));
  hpEl.maliketh.textContent = Math.max(0, Math.floor(state.maliketh.hp));

  if (state.maliketh.hp <= 0) {
    state.over = true;
    ending.classList.remove('hidden');
    ending.innerHTML = '🎉 <strong>Victoria:</strong> Vivi y Wisky derrotaron a Maliketh en 2D y rescataron a Giovanny. Ahora el reino es una fiesta de abrazos, corazones y croquetas premium.';
    log('¡Final feliz desbloqueado!');
  }

  if (state.vivi.hp <= 0 && state.wisky.hp <= 0) {
    state.over = true;
    ending.classList.remove('hidden');
    ending.innerHTML = '💔 Maliketh ganó esta ronda. Reinicia para volver más fuertes y más melosos.';
    log('Derrota temporal del equipo romántico.');
  }
}

function drawBackground() {
  const g = ctx.createLinearGradient(0, 0, 0, world.height);
  g.addColorStop(0, '#5f20d0');
  g.addColorStop(0.45, '#ff63bb');
  g.addColorStop(1, '#ffd26e');
  ctx.fillStyle = g;
  ctx.fillRect(0, 0, world.width, world.height);

  ctx.fillStyle = '#89ffde';
  ctx.fillRect(0, 0, world.width, 20);

  ctx.fillStyle = '#7fdd6a';
  ctx.fillRect(0, 430, world.width, 110);

  world.flowers.forEach((f) => {
    ctx.fillStyle = f.c ? '#fff0ff' : '#71ffe9';
    ctx.fillRect(f.x, f.y, 4, 4);
    ctx.fillStyle = '#ff5ea8';
    ctx.fillRect(f.x + 1, f.y - 2, 2, 2);
  });

  ctx.fillStyle = '#54407a';
  ctx.fillRect(700, 210, 150, 220);
  ctx.fillStyle = '#ecb5ff';
  ctx.fillRect(760, 260, 25, 25);
  ctx.fillStyle = '#fff089';
  ctx.fillRect(788, 220, 9, 9);
}

function drawChars() {
  drawSprite(ctx, sprites.vivi, palettes.vivi, state.vivi.x, state.vivi.y, 8);
  drawSprite(ctx, sprites.wisky, palettes.wisky, state.wisky.x, state.wisky.y, 8);
  drawSprite(ctx, sprites.maliketh, palettes.maliketh, state.maliketh.x, state.maliketh.y, 8);

  state.attacks.forEach((a) => {
    ctx.fillStyle = a.owner === 'vivi' ? '#fff3aa' : '#7cfaff';
    ctx.fillRect(a.x, a.y, a.w, a.h);
  });

  ctx.fillStyle = '#ffffff';
  ctx.font = '20px VT323';
  ctx.fillText('Vivi', state.vivi.x, state.vivi.y - 8);
  ctx.fillText('Wisky', state.wisky.x, state.wisky.y - 8);
  ctx.fillText('Maliketh', state.maliketh.x - 8, state.maliketh.y - 8);
}

function loop() {
  drawBackground();
  update();
  drawChars();
  requestAnimationFrame(loop);
}

window.addEventListener('keydown', (e) => {
  const k = e.key.toLowerCase();
  state.keys.add(k);
  if (k === ' ') {
    e.preventDefault();
    spawnAttack('vivi');
  }
  if (k === 'enter') {
    e.preventDefault();
    spawnAttack('wisky');
  }
});

window.addEventListener('keyup', (e) => {
  state.keys.delete(e.key.toLowerCase());
});

restart.addEventListener('click', () => {
  state.over = false;
  state.vivi = { x: 120, y: 380, hp: 100, speed: 2.6 };
  state.wisky = { x: 185, y: 400, hp: 100, speed: 2.9 };
  state.maliketh = { x: 760, y: 360, hp: 220, dir: 1 };
  state.attacks = [];
  ending.classList.add('hidden');
  logBox.innerHTML = '';
  log('Nueva partida: rescate romántico en marcha.');
});

log('Empieza la aventura 2D: mueve a Vivi y Wisky, y derriba a Maliketh.');
loop();
