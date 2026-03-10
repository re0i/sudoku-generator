async function fetchPuzzle() {
    const size = document.getElementById("size").value;
    const difficulty = document.getElementById("difficulty").value;
    const showSolution = document.getElementById("showSolution").checked;

    const url = `http://localhost:18080/new?size=${size}&difficulty=${difficulty}&showSolution=${showSolution}`;

    const res = await fetch(url);
    const data = await res.json();

    renderGrid(data.puzzle, data.size);
    if (showSolution) {
        renderSol(data.solution, data.size);
    } else {
        document.getElementById("solGrid").innerHTML = "";
    }
}

function renderGrid(puzzle, size) {
    const container = document.getElementById("grid");
    container.innerHTML = "";

    const table = document.createElement("table");

    for (let i = 0; i < size; i++) {
        const row = document.createElement("tr");
        for (let j = 0; j < size; j++) {
            const cell = document.createElement("td");
            const val = puzzle[i][j];

            if (val !== 0) {
                cell.textContent = val;
                cell.classList.add("clue");
            } else {
                const input = document.createElement("input");
                input.type = "text";
                input.maxLength = 1;
                cell.appendChild(input);
            }
            row.appendChild(cell);
        }
        table.appendChild(row);
    }
    container.appendChild(table);
}

function renderSol(solution, size) {
    const container = document.getElementById("solGrid");
    container.innerHTML = "";

    const table = document.createElement("table");

    for (let i = 0; i < size; i++) {
        const row = document.createElement("tr");
        for (let j = 0; j < size; j++) {
            const cell = document.createElement("td");
            cell.textContent = solution[i][j];
            row.appendChild(cell);
        }
        table.appendChild(row);
    }
    container.appendChild(table);
}