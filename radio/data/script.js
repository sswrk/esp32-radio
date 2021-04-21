let currentStation = '302.1';
var xmlhttp = new XMLHttpRequest();

function updateCurrentStation(newStation) {
    currentStation = newStation;
    document.getElementById('current-station').innerHTML = currentStation;
}

function executeLoop() {
    updateCurrentStation(currentStation);
}

async function setPrevStation() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState = 4 && this.status == 200) {
            updateCurrentStation(this.responseText)
        }
    }
    xmlhttp.open("GET", "192.168.1.24/station/prev", true);
    xmlhttp.send()
}

function togglePlayState() {

}

function setNextStation() {

}

executeLoop()