export const palettes = {
  vivi: {
    1: '#1e1a18', // outline
    2: '#f4d9c7', // skin base caucásica
    3: '#eec8af', // skin shade
    4: '#5f3b28', // cabello café
    5: '#8f5d3d', // cabello reflejo
    6: '#c79a6d', // reflejo claro
    7: '#2f2f38', // lentes
    8: '#f7e8ff', // blusa clara
    9: '#ff74bc', // chaqueta rosa
    A: '#7f4bcb', // falda/pantalón
    B: '#ffffff', // brillo
    C: '#ffb3db', // detalle romántico
  },
  wisky: {
    1: '#171717', // outline
    2: '#0f0f0f', // pelaje negro tuxedo
    3: '#f7f7f7', // pelaje blanco
    4: '#74dfff', // ojos brillantes
    5: '#ffd06f', // collar dorado
    6: '#fca5d3', // nariz
    7: '#cbf7ff', // brillo
    8: '#2fe08d', // acento energético
  },
  maliketh: {
    1: '#18131c', // outline
    2: '#fff5ec', // crema fluffy
    3: '#f1dcc9', // sombra crema
    4: '#8f7b73', // tabby point
    5: '#6e5f59', // tabby oscuro
    6: '#67bcff', // ojos azules
    7: '#9f5be2', // aura malvada
    8: '#ff79b5', // corona/adorno
    9: '#ffffff', // brillo
  },
  giovanny: {
    1: '#1d1b1d', // outline
    2: '#efd3bc', // piel
    3: '#2a2a2f', // pelo negro
    4: '#121216', // barba
    5: '#93d9ff', // lentes reflejo
    6: '#426de3', // ropa
    7: '#6fe2c5', // detalle
    8: '#ffffff', // brillo
  }
};

export const sprites = {
  vivi: [
    '0000000011110000',
    '0000001144411000',
    '0000011445541100',
    '0000114455554110',
    '0000144222245410',
    '0001422777222410',
    '0014222BB2222241',
    '0014222222222241',
    '0019422222222291',
    '0199992882229991',
    '0199999888999991',
    '0019999AA9999910',
    '0001AAA11AAA1100',
    '0011AAA11AAA1110',
    '0110001000100011',
    '1100001000100001'
  ],
  wisky: [
    '0000011001100000',
    '0000111111110000',
    '0001122222221100',
    '0011222222222110',
    '0112233333332211',
    '1122333444333221',
    '1122333333333221',
    '1122333663333221',
    '1122333333333221',
    '1122233333332221',
    '0111123333321110',
    '0011153333351100',
    '0011133333331100',
    '0111333383333110',
    '0110001111100011',
    '1100001000100001'
  ],
  maliketh: [
    '0000000777700000',
    '0000007888870000',
    '0000172222271000',
    '0001722222227100',
    '0017224444422710',
    '0172245665422271',
    '1722242662422221',
    '1722242222222221',
    '1722222333222221',
    '1722222333222221',
    '0172222222222271',
    '0017222222222710',
    '0001722333227100',
    '0000172333271000',
    '0000017111170000',
    '0000001100110000'
  ],
  giovanny: [
    '0000000111100000',
    '0000001333310000',
    '0000013333331000',
    '0000133222333100',
    '0001332555223310',
    '0001322222223310',
    '0013222244222331',
    '0013222444422331',
    '0011334444443311',
    '0001333222333100',
    '0001666666663100',
    '0011666666666110',
    '0011666111666110',
    '0111666111666111',
    '1100110000110011',
    '1000110000110001'
  ],
};

export function drawSprite(ctx, sprite, palette, x, y, scale = 4) {
  sprite.forEach((row, j) => {
    [...row].forEach((cell, i) => {
      const idx = cell === '0' ? 0 : cell;
      if (idx === 0) return;
      ctx.fillStyle = palette[idx] ?? '#fff';
      ctx.fillRect(x + i * scale, y + j * scale, scale, scale);
    });
  });
}
