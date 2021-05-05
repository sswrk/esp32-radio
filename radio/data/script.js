let currentStation = '302.1';
let favouritesStationsArr = [];
let savedStationsArr = [];
let isPlaying = false;

let xmlhttp = new XMLHttpRequest();

function fetchStations() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr = JSON.parse(this.responseText);
        }
    }
    xmlhttp.open("GET", "/stations", true);
    xmlhttp.send()
}

function updateCurrentStation(newStation) {
    currentStation = newStation;
    document.getElementById('current-station').innerHTML = currentStation;
}


function addStationToFavorites(id) {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            this.savedStationsArr.find((s) => s.id === id).isFavourite = true;
        }
    }
    xmlhttp.open("POST", "/stations/favourites", true);
    xmlhttp.send('id=' + id);
}

async function selectPreviousStation(id) {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStation(this.responseText)
        }
    }
    xmlhttp.open("POST", "/station/set-station", true);
    xmlhttp.send('id=' + id)
}

function togglePlayState() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            isPlaying = !isPlaying;
        }
    }

    xmlhttp.open("GET", "/control/toggle-play", true);
    xmlhttp.send();
}

function selectNextStation() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStation(this.responseText)
        }
    }
    xmlhttp.open("POST", "/station/set-station", true);
    xmlhttp.send('id=' + id)
}

function makeMoreQuiet() {
    xmlhttp.onreadystatechange = () => {
    };
    xmlhttp.open("GET", "/control/more-quiet", true)
    xmlhttp.send();
}

function makeLouder() {
    xmlhttp.onreadystatechange = () => {
    };
    xmlhttp.open("GET", "/control/louder", true)
    xmlhttp.send();
}
}

function executeLoop() {
    fetchStations();
    updateCurrentStation(currentStation);
}

executeLoop()