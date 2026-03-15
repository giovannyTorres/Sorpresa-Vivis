export const palettes = {
  vivi: ['#000000', '#fefefe', '#ff92d6', '#f56fb2', '#7c4ad7'],
  wisky: ['#000000', '#ffffff', '#1a1a1a', '#ffe56d', '#57e8ff'],
  maliketh: ['#000000', '#ffe9ff', '#ff5b93', '#9f1c64', '#4c0830'],
};

export const sprites = {
  vivi: [
    '00011000',
    '00133200',
    '01333320',
    '01344320',
    '00144300',
    '00322300',
    '02300320',
    '22000222'
  ],
  wisky: [
    '22000222',
    '22222222',
    '21111212',
    '21111212',
    '21111112',
    '21141112',
    '02111120',
    '00222000'
  ],
  maliketh: [
    '00444400',
    '04333340',
    '43322334',
    '43311334',
    '43333334',
    '44322344',
    '04444440',
    '00400400'
  ]
};

export function drawSprite(ctx, sprite, palette, x, y, scale = 8) {
  sprite.forEach((row, j) => {
    [...row].forEach((cell, i) => {
      const idx = Number(cell);
      if (idx === 0) return;
      ctx.fillStyle = palette[idx] ?? '#fff';
      ctx.fillRect(x + i * scale, y + j * scale, scale, scale);
    });
  });
}
