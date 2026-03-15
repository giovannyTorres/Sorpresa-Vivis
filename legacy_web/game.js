import { palettes, sprites, drawSprite } from './sprites.js';

const canvas = document.getElementById('game');
const hpEl = {
  vivi: document.getElementById('viviHp'),
  wisky: document.getElementById('wiskyHp'),
  maliketh: document.getElementById('malikethHp'),
};
const logBox = document.getElementById('log');
const ending = document.getElementById('ending');
const restart = document.getElementById('restart');

function failBoot(message) {
  ending.classList.remove('hidden');
  ending.textContent = message;
}

if (!canvas) {
  throw new Error('No se encontró #game');
}

const ctx = canvas.getContext('2d');
if (!ctx) {
  failBoot('Error: no se pudo inicializar el canvas.');
  throw new Error('Canvas 2D context unavailable');
}

const TILE = 24;
const MAP_W = Math.floor(canvas.width / TILE);
const MAP_H = Math.floor(canvas.height / TILE);

const world = {
  width: canvas.width,
  height: canvas.height,
  time: 0,
  grassNoise: Array.from({ length: MAP_W }, () => rand(0, 2)),
  flowers: Array.from({ length: 110 }, () => ({ x: rand(1, MAP_W - 2), y: rand(14, MAP_H - 1), c: rand(0, 2) })),
};

const state = {
  over: false,
  vivi: { x: 120, y: 356, hp: 100, speed: 2.8 },
  wisky: { x: 210, y: 382, hp: 100, speed: 3 },
  maliketh: { x: 730, y: 330, hp: 220, dir: 1 },
  giovanny: { x: 824, y: 278, rescued: false },
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
    state.attacks.push({ owner, x: state.vivi.x + 58, y: state.vivi.y + 28, vx: 6.8, w: 18, h: 8, dmg: rand(14, 23), color: '#ffd6f4', label: 'Ráfaga de Puños Tiernos' });
    log('Vivi usa Ráfaga de Puños Tiernos ✨');
  } else {
    state.attacks.push({ owner, x: state.wisky.x + 56, y: state.wisky.y + 30, vx: 7.2, w: 12, h: 7, dmg: rand(13, 25), color: '#7aefff', label: 'Arañazo de Wisky' });
    log('Wisky (tuxedo) usa Arañazo ⚡');
  }
}

function enemyAttack() {
  if (state.over) return;
  const target = Math.random() < 0.5 ? state.vivi : state.wisky;
  const dmg = rand(6, 13);
  target.hp -= dmg;
  log(`Maliketh contraataca con Pelusa Maldita: ${dmg} daño.`);
}

function update() {
  if (state.over) return;
  world.time += 0.016;

  if (state.keys.has('w')) state.vivi.y -= state.vivi.speed;
  if (state.keys.has('s')) state.vivi.y += state.vivi.speed;
  if (state.keys.has('a')) state.vivi.x -= state.vivi.speed;
  if (state.keys.has('d')) state.vivi.x += state.vivi.speed;

  if (state.keys.has('arrowup')) state.wisky.y -= state.wisky.speed;
  if (state.keys.has('arrowdown')) state.wisky.y += state.wisky.speed;
  if (state.keys.has('arrowleft')) state.wisky.x -= state.wisky.speed;
  if (state.keys.has('arrowright')) state.wisky.x += state.wisky.speed;

  state.vivi.x = clamp(state.vivi.x, 20, world.width - 90);
  state.vivi.y = clamp(state.vivi.y, 240, world.height - 80);
  state.wisky.x = clamp(state.wisky.x, 20, world.width - 90);
  state.wisky.y = clamp(state.wisky.y, 240, world.height - 80);

  state.maliketh.x += state.maliketh.dir * 1.25;
  if (state.maliketh.x < 640 || state.maliketh.x > 860) state.maliketh.dir *= -1;

  state.attacks.forEach((a) => { a.x += a.vx; });
  state.attacks = state.attacks.filter((a) => a.x < world.width + 20);

  const enemyRect = { x: state.maliketh.x, y: state.maliketh.y, w: 70, h: 54 };
  state.attacks = state.attacks.filter((a) => {
    const atk = { x: a.x, y: a.y, w: a.w, h: a.h };
    if (!rectHit(atk, enemyRect)) return true;
    state.maliketh.hp -= a.dmg;
    log(`${a.label} conecta por ${a.dmg}.`);
    if (Math.random() < 0.35) enemyAttack();
    return false;
  });

  hpEl.vivi.textContent = String(Math.max(0, Math.floor(state.vivi.hp)));
  hpEl.wisky.textContent = String(Math.max(0, Math.floor(state.wisky.hp)));
  hpEl.maliketh.textContent = String(Math.max(0, Math.floor(state.maliketh.hp)));

  if (state.maliketh.hp <= 0) {
    state.over = true;
    state.giovanny.rescued = true;
    ending.classList.remove('hidden');
    ending.innerHTML = '🎉 <strong>Victoria:</strong> Maliketh fue derrotada y Giovanny fue rescatado.';
    log('Final feliz desbloqueado.');
  }

  if (state.vivi.hp <= 0 && state.wisky.hp <= 0) {
    state.over = true;
    ending.classList.remove('hidden');
    ending.innerHTML = '💔 Derrota temporal. Reinicia y vuelve a intentarlo.';
    log('Maliketh ganó esta ronda.');
  }
}

function drawPixelRect(x, y, w, h, c) {
  ctx.fillStyle = c;
  ctx.fillRect(Math.floor(x), Math.floor(y), Math.floor(w), Math.floor(h));
}

function drawBackground() {
  // cielo por bandas estilo RPG
  drawPixelRect(0, 0, world.width, 120, '#2f2c95');
  drawPixelRect(0, 120, world.width, 90, '#5b36b5');
  drawPixelRect(0, 210, world.width, 80, '#9465dd');
  drawPixelRect(0, 290, world.width, 50, '#c875d9');

  // estrellas
  for (let i = 0; i < 46; i += 1) {
    const tw = Math.sin(world.time * 2 + i) > 0 ? '#ffffff' : '#d6e8ff';
    drawPixelRect((i * 53) % world.width, 14 + ((i * 37) % 140), 3, 3, tw);
  }

  // montañas pixeladas
  drawPixelRect(80, 220, 180, 120, '#5f4ea3');
  drawPixelRect(120, 190, 100, 40, '#7f6ec3');
  drawPixelRect(270, 210, 210, 130, '#584498');
  drawPixelRect(310, 185, 120, 30, '#7861bc');

  // suelo tipo ruta pokemon
  drawPixelRect(0, 340, world.width, 200, '#6fce70');
  for (let x = 0; x < MAP_W; x += 1) {
    const h = world.grassNoise[x];
    drawPixelRect(x * TILE, 340, TILE - 2, 8 + h, '#59b35e');
  }

  // camino
  drawPixelRect(0, 432, world.width, 56, '#ccb27a');
  for (let i = 0; i < MAP_W; i += 2) {
    drawPixelRect(i * TILE, 432 + ((i % 4) ? 2 : 0), TILE - 4, 10, '#b99b65');
  }

  // árboles angulares
  for (let i = 0; i < 8; i += 1) {
    const tx = 40 + i * 95;
    drawPixelRect(tx + 18, 295, 10, 40, '#6f4b2a');
    drawPixelRect(tx, 268, 46, 30, '#2f9b4f');
    drawPixelRect(tx + 6, 248, 34, 24, '#38b45c');
  }

  // flores
  const flowerColors = ['#ffb3db', '#8ffff0', '#ffe38f'];
  world.flowers.forEach((f, idx) => {
    const sway = Math.sin(world.time * 4 + idx) > 0 ? 1 : 0;
    const px = f.x * TILE + 8 + sway;
    const py = f.y * TILE;
    drawPixelRect(px + 1, py + 3, 2, 5, '#2d964d');
    drawPixelRect(px, py, 4, 4, flowerColors[f.c]);
    drawPixelRect(px + 1, py + 1, 2, 2, '#ff5fb2');
  });

  // torre
  drawPixelRect(730, 172, 180, 170, '#4b3d74');
  drawPixelRect(716, 336, 208, 12, '#6a56a4');
  drawPixelRect(793, 280, 52, 62, '#2b1d4a');
  for (let i = 0; i < 6; i += 1) {
    drawPixelRect(744 + i * 26, 206, 14, 14, i % 2 ? '#f5c8fe' : '#fff2a8');
  }
}

function drawCharacters() {
  const bob1 = Math.sin(world.time * 9) * 1;
  const bob2 = Math.sin(world.time * 11) * 1;
  const bob3 = Math.sin(world.time * 4) * 1;

  drawSprite(ctx, sprites.vivi, palettes.vivi, state.vivi.x, state.vivi.y + bob1, 3);
  drawSprite(ctx, sprites.wisky, palettes.wisky, state.wisky.x, state.wisky.y + bob2, 3);
  drawSprite(ctx, sprites.maliketh, palettes.maliketh, state.maliketh.x, state.maliketh.y + bob3, 3);

  const gx = state.giovanny.rescued ? 635 : state.giovanny.x;
  const gy = state.giovanny.rescued ? 350 : state.giovanny.y;
  drawSprite(ctx, sprites.giovanny, palettes.giovanny, gx, gy, 3);

  if (!state.giovanny.rescued) {
    ctx.strokeStyle = '#ff96cb';
    ctx.lineWidth = 2;
    ctx.strokeRect(gx - 6, gy - 4, 82, 64);
  }

  state.attacks.forEach((a) => {
    drawPixelRect(a.x, a.y, a.w, a.h, a.color);
    drawPixelRect(a.x - 2, a.y + 1, 2, 2, '#ffffff');
  });

  ctx.font = '14px "Press Start 2P"';
  ctx.fillStyle = '#fffbe8';
  ctx.fillText('Vivi', state.vivi.x - 2, state.vivi.y - 8);
  ctx.fillText('Wisky', state.wisky.x - 4, state.wisky.y - 8);
  ctx.fillText('Maliketh', state.maliketh.x - 10, state.maliketh.y - 8);
}

function loop() {
  drawBackground();
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
  world.time = 0;
  state.vivi = { x: 120, y: 356, hp: 100, speed: 2.8 };
  state.wisky = { x: 210, y: 382, hp: 100, speed: 3 };
  state.maliketh = { x: 730, y: 330, hp: 220, dir: 1 };
  state.giovanny = { x: 824, y: 278, rescued: false };
  state.attacks = [];
  ending.classList.add('hidden');
  logBox.innerHTML = '';
  hpEl.vivi.textContent = '100';
  hpEl.wisky.textContent = '100';
  hpEl.maliketh.textContent = '220';
  log('Nueva partida iniciada.');
});

log('Juego cargado correctamente: escenario y personajes activos.');
loop();
