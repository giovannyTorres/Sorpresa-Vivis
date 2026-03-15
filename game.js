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
  flowers: Array.from({ length: 160 }, () => ({
    x: rand(0, canvas.width),
    y: rand(330, canvas.height - 10),
    tone: rand(0, 2),
  })),
  stars: Array.from({ length: 70 }, () => ({ x: rand(0, canvas.width), y: rand(0, 170), s: rand(1, 3) })),
  clouds: Array.from({ length: 7 }, () => ({ x: rand(0, canvas.width), y: rand(30, 140), w: rand(80, 160) })),
};

const state = {
  over: false,
  time: 0,
  vivi: { x: 130, y: 392, hp: 100, speed: 2.6 },
  wisky: { x: 220, y: 406, hp: 100, speed: 2.9 },
  maliketh: { x: 760, y: 360, hp: 220, dir: 1 },
  giovanny: { x: 845, y: 289, rescued: false },
  attacks: [],
  keys: new Set(),
};

function rand(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function clamp(v, min, max) {
  return Math.max(min, Math.min(max, v));
}

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
    state.attacks.push({
      owner,
      x: state.vivi.x + 48,
      y: state.vivi.y + 28,
      vx: 6.5,
      w: 20,
      h: 12,
      dmg: rand(14, 24),
      label: 'Ráfaga de Puños Tiernos',
      color: '#ffd3f1',
    });
    log('Vivi (cabello café con reflejos y lentes) desata su Ráfaga de Puños Tiernos.');
  } else {
    state.attacks.push({
      owner,
      x: state.wisky.x + 46,
      y: state.wisky.y + 26,
      vx: 7,
      w: 14,
      h: 10,
      dmg: rand(12, 26),
      label: 'Arañazo de Wisky',
      color: '#7bf6ff',
    });
    log('Wisky, tuxedo hiperactivo, lanza un arañazo chispeante y latoso.');
  }
}

function enemyAttack() {
  if (state.over) return;

  const target = Math.random() < 0.5 ? state.vivi : state.wisky;
  const damage = rand(6, 14);
  target.hp -= damage;
  log(`Maliketh (gato tabby point esponjoso, ojos azules) contraataca con pelusa maldita: ${damage} daño.`);
}

function update() {
  if (state.over) return;

  state.time += 0.016;

  if (state.keys.has('w')) state.vivi.y -= state.vivi.speed;
  if (state.keys.has('s')) state.vivi.y += state.vivi.speed;
  if (state.keys.has('a')) state.vivi.x -= state.vivi.speed;
  if (state.keys.has('d')) state.vivi.x += state.vivi.speed;

  if (state.keys.has('arrowup')) state.wisky.y -= state.wisky.speed;
  if (state.keys.has('arrowdown')) state.wisky.y += state.wisky.speed;
  if (state.keys.has('arrowleft')) state.wisky.x -= state.wisky.speed;
  if (state.keys.has('arrowright')) state.wisky.x += state.wisky.speed;

  state.vivi.x = clamp(state.vivi.x, 25, world.width - 90);
  state.vivi.y = clamp(state.vivi.y, 256, world.height - 80);
  state.wisky.x = clamp(state.wisky.x, 25, world.width - 90);
  state.wisky.y = clamp(state.wisky.y, 256, world.height - 80);

  state.maliketh.x += state.maliketh.dir * 1.4;
  if (state.maliketh.x < 620 || state.maliketh.x > 860) state.maliketh.dir *= -1;

  state.attacks.forEach((a) => {
    a.x += a.vx;
  });
  state.attacks = state.attacks.filter((a) => a.x < world.width + 24);

  const enemyRect = { x: state.maliketh.x, y: state.maliketh.y, w: 64, h: 64 };

  state.attacks = state.attacks.filter((a) => {
    const hitbox = { x: a.x, y: a.y, w: a.w, h: a.h };
    if (rectHit(hitbox, enemyRect)) {
      state.maliketh.hp -= a.dmg;
      log(`${a.label} conecta con ${a.dmg} de daño.`);
      if (Math.random() < 0.37) enemyAttack();
      return false;
    }
    return true;
  });

  hpEl.vivi.textContent = Math.max(0, Math.floor(state.vivi.hp));
  hpEl.wisky.textContent = Math.max(0, Math.floor(state.wisky.hp));
  hpEl.maliketh.textContent = Math.max(0, Math.floor(state.maliketh.hp));

  if (state.maliketh.hp <= 0) {
    state.over = true;
    state.giovanny.rescued = true;
    ending.classList.remove('hidden');
    ending.innerHTML = '🎉 <strong>Victoria:</strong> Vivi y Wisky derrotaron a Maliketh. Giovanny (pelo negro, barba y lentes) fue rescatado y el reino se llenó de amor pixelado.';
    log('Final feliz: Giovanny sale de la torre, abraza al equipo y todo brilla en 8 bits.');
  }

  if (state.vivi.hp <= 0 && state.wisky.hp <= 0) {
    state.over = true;
    ending.classList.remove('hidden');
    ending.innerHTML = '💔 Maliketh ganó esta ronda. Reinicia para volver con más ternura, más estilo y más garra.';
    log('Derrota temporal del escuadrón romántico.');
  }
}

function drawSky() {
  const sky = ctx.createLinearGradient(0, 0, 0, world.height);
  sky.addColorStop(0, '#40208f');
  sky.addColorStop(0.33, '#6f35cb');
  sky.addColorStop(0.62, '#ff6cc4');
  sky.addColorStop(1, '#ffcf6d');
  ctx.fillStyle = sky;
  ctx.fillRect(0, 0, world.width, world.height);

  world.stars.forEach((s, i) => {
    const pulse = 0.35 + Math.abs(Math.sin(state.time * 2 + i)) * 0.65;
    ctx.fillStyle = `rgba(255,255,255,${pulse.toFixed(2)})`;
    ctx.fillRect(s.x, s.y, s.s, s.s);
  });

  world.clouds.forEach((c, i) => {
    const drift = ((state.time * (8 + i)) % (world.width + c.w * 2)) - c.w;
    const x = (c.x + drift) % (world.width + c.w) - c.w;
    ctx.fillStyle = 'rgba(255, 226, 249, 0.35)';
    ctx.fillRect(x, c.y, c.w, 20);
    ctx.fillRect(x + 12, c.y - 10, c.w * 0.6, 18);
    ctx.fillRect(x + 26, c.y + 10, c.w * 0.45, 14);
  });
}

function drawKingdom() {
  ctx.fillStyle = '#7de27a';
  ctx.fillRect(0, 328, world.width, world.height - 328);

  ctx.fillStyle = '#68bf68';
  for (let x = 0; x < world.width; x += 24) {
    const h = 9 + Math.sin((x + state.time * 40) * 0.02) * 3;
    ctx.fillRect(x, 328, 16, h);
  }

  world.flowers.forEach((f, idx) => {
    const stemY = f.y;
    const sway = Math.sin(state.time * 4 + idx) * 1.2;
    ctx.fillStyle = '#2ea654';
    ctx.fillRect(f.x + sway, stemY, 1, 5);
    const petals = ['#ffe6fa', '#8ffff0', '#ffd178'];
    ctx.fillStyle = petals[f.tone];
    ctx.fillRect(f.x - 1 + sway, stemY - 2, 4, 3);
    ctx.fillStyle = '#ff5cb1';
    ctx.fillRect(f.x + sway, stemY - 1, 2, 2);
  });

  // torre del reino de pelos de gato
  ctx.fillStyle = '#4f3a72';
  ctx.fillRect(735, 170, 170, 175);
  ctx.fillStyle = '#6b4e9f';
  ctx.fillRect(720, 335, 200, 16);

  for (let i = 0; i < 6; i += 1) {
    ctx.fillStyle = i % 2 ? '#f8c8ff' : '#fff08f';
    ctx.fillRect(748 + i * 24, 204, 14, 14);
  }

  ctx.fillStyle = '#2d1b49';
  ctx.fillRect(796, 290, 44, 55);

  // mechones/pelusa mágica alrededor de la torre
  for (let i = 0; i < 16; i += 1) {
    const px = 700 + i * 18 + Math.sin(state.time * 2 + i) * 5;
    const py = 155 + (i % 6) * 12;
    ctx.fillStyle = 'rgba(255, 229, 245, 0.7)';
    ctx.fillRect(px, py, 6, 4);
  }
}

function drawCharacters() {
  const bobVivi = Math.sin(state.time * 8) * 1.5;
  const bobWisky = Math.sin(state.time * 11) * 1.7;
  const bobMaliketh = Math.sin(state.time * 3) * 2;

  drawSprite(ctx, sprites.vivi, palettes.vivi, state.vivi.x, state.vivi.y + bobVivi, 4);
  drawSprite(ctx, sprites.wisky, palettes.wisky, state.wisky.x, state.wisky.y + bobWisky, 4);
  drawSprite(ctx, sprites.maliketh, palettes.maliketh, state.maliketh.x, state.maliketh.y + bobMaliketh, 4);

  const gioX = state.giovanny.rescued ? 635 : state.giovanny.x;
  const gioY = state.giovanny.rescued ? 360 : state.giovanny.y;
  drawSprite(ctx, sprites.giovanny, palettes.giovanny, gioX, gioY + Math.sin(state.time * 6) * 1.4, 4);

  if (!state.giovanny.rescued) {
    ctx.strokeStyle = '#ff93c8';
    ctx.lineWidth = 2;
    ctx.strokeRect(gioX - 8, gioY - 6, 80, 82);
  }

  state.attacks.forEach((a, i) => {
    ctx.fillStyle = a.color;
    ctx.fillRect(a.x, a.y, a.w, a.h);
    ctx.fillStyle = '#ffffff';
    ctx.fillRect(a.x - 2, a.y + ((i % 2) ? 2 : 4), 3, 3);
  });

  ctx.font = '18px "Press Start 2P"';
  ctx.fillStyle = '#fffaf0';
  ctx.fillText('Vivi', state.vivi.x - 4, state.vivi.y - 8);
  ctx.fillText('Wisky', state.wisky.x - 8, state.wisky.y - 8);
  ctx.fillText('Maliketh', state.maliketh.x - 12, state.maliketh.y - 8);
  ctx.fillText('Giovanny', gioX - 12, gioY - 10);
}

function loop() {
  drawSky();
  drawKingdom();
  update();
  drawCharacters();
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
  state.time = 0;
  state.vivi = { x: 130, y: 392, hp: 100, speed: 2.6 };
  state.wisky = { x: 220, y: 406, hp: 100, speed: 2.9 };
  state.maliketh = { x: 760, y: 360, hp: 220, dir: 1 };
  state.giovanny = { x: 845, y: 289, rescued: false };
  state.attacks = [];
  ending.classList.add('hidden');
  logBox.innerHTML = '';
  hpEl.vivi.textContent = '100';
  hpEl.wisky.textContent = '100';
  hpEl.maliketh.textContent = '220';
  log('Nueva partida: arte pulido y rescate en marcha.');
});

log('Inicio: Vivi, Wisky y su estilo 8-bits avanzan hacia la torre de Maliketh para liberar a Giovanny.');
loop();
