const CELL_SIZE = 50;
const IMG_PROB = 0.05;

const DIGIT_COLORS = ['#c8b8a2','#a8c4b8','#b8a8c4','#c4b8a8','#a2b8c8','#c4a8b8','#b8c4a8','#c8a8a2','#b2b8c0'];
const IMG_SEED = ['']; // To-do: Add imgs

function buildGrid() {
    const container = document.getElementById('grid-container');
    const W = window.innerWidth;
    const H = window.innerHeight;
 
    const cols = Math.round(W / CELL_SIZE);
    const rows = Math.round(H / CELL_SIZE);
 
    container.style.gridTemplateColumns = `repeat(${cols}, 1fr)`;
    container.style.gridTemplateRows = `repeat(${rows}, 1fr)`;
    container.innerHTML = '';

    for (let i = 0; i < cols * rows; i++) {
        const cell = document.createElement('div');
        cell.className = 'cell';

        /* Front face - the defualt visible tile */
        const front = document.createElement('div');
        front.className = 'cell-front';
        front.innerHTML = '';

        /* Back face — revealed on hover */
        const back = document.createElement('div');
        back.className = 'cell-back';
 
        if (Math.random() < IMG_PROB) {
            const img = document.createElement('img');
            img.src = `./`; // To-do: Add paths to the imgs
            img.loading = 'eager';
            img.alt = '';
            back.style.background = '#1a1a1a';
            back.appendChild(img);
        } else {
            const num = Math.floor(Math.random() * 9) + 1;
            back.textContent = num;
            back.style.color = DIGIT_COLORS[num - 1];
            back.style.background = '#161616';
        }
    
        cell.appendChild(back);
        cell.appendChild(front);
        container.appendChild(cell);
    }
}
    
buildGrid();
let resizeTimer;
window.addEventListener('resize', () => {
    clearTimeout(resizeTimer);
    resizeTimer = setTimeout(buildGrid, 150);
});
