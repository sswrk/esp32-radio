let favouritesStationsArr = [];
let savedStationsArr = [
    {
        id: 32, isFavourite: false, name: 'Andrzejska'
    },
    {
        id: 31, isFavourite: false, name: 'Mother'
    },
    {
        id: 30, isFavourite: false, name: 'Father'
    },
    {
        id: 29, isFavourite: false, name: 'Sister'
    }
];
let currentStation = savedStationsArr[0];
let isPlaying = false;
let xmlhttp = new XMLHttpRequest();

async function fetchStations() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr = JSON.parse(this.responseText);
            currentStation = savedStationsArr[0];
            initIsFavourite();
        }
    }
    xmlhttp.open("GET", "/stations", true);
    xmlhttp.send()
}

async function initIsFavourite() {
    (currentStation.isFavourite) ? $('#add-favourite-heart').addClass('d-none') : $('#delete-favourite-heart').addClass('d-none');
}

async function updateCurrentStation(newStation) {
    currentStation = newStation;
    $('#current-station').html(currentStation.name);
}

async function toggleStationIsFavourite() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            currentStation.isFavourite = !currentStation.isFavourite;
            if (currentStation.isFavourite) {
                $('#add-favourite-heart').addClass('d-none');
                $('#delete-favourite-heart').removeClass('d-none');
            } else {
                $('#add-favourite-heart').removeClass('d-none');
                $('#delete-favourite-heart').addClass('d-none');
            }
        }
    }
    xmlhttp.open(currentStation.isFavourite ? "DELETE" : "POST", "/stations/favourites", true);
    xmlhttp.send('id=' + currentStation.id);

}

async function selectPreviousStation() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStation(this.responseText)
        }
    }

    let nextStationIndex = savedStationsArr.findIndex((s) => s.id === currentStation.id) - 1;
    if (nextStationIndex < 0) nextStationIndex = savedStationsArr.length - 1;
    const nextStation = savedStationsArr[nextStationIndex];

    xmlhttp.open("POST", "/station/set-station", true);
    xmlhttp.send('id=' + nextStation.id)
}

function togglePlayState() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            isPlaying = !isPlaying;
            const playButton = $('#play-button');
            playButton.html(isPlaying ? 'Pause' : 'Play');
            playButton.removeClass(isPlaying ? 'btn-success' : 'btn-danger');
            playButton.addClass(isPlaying ? 'btn-danger' : 'btn-success');
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
    let nextStationIndex = savedStationsArr.findIndex((s) => s.id === currentStation.id) + 1;
    if (nextStationIndex >= savedStationsArr.length) nextStationIndex = 0;
    const nextStation = savedStationsArr[nextStationIndex];
    xmlhttp.open("POST", "/station/set-station", true);
    xmlhttp.send('id=' + nextStation.id)
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

function addStationToSaved() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr.push(JSON.parse(this.responseText));
            $('#name').html('');
            $('#host').html('');
            $('#port').html('');
            $('#path').html('');

            $(document).ready(function () {
                $("#liveToast").toast('show');
            })
        }
    }
    const formData = new FormData();
    formData.append('name', $('#name').value);
    formData.append('host', $('#host').value);
    formData.append('port', $('#port').value);
    formData.append('path', $('#path').value);

    xmlhttp.open("POST", "/stations", true);
    xmlhttp.send(formData)
}

function addStationsToDOM() {

    // TODO: finish dynamic stations selection and showing
    // const savedStationsList = $('#saved-stations');
    // savedStationsArr.forEach((s) => {
    //     const newChild = $("a").append(s.name).addClass('list-group-item')
    //         .addClass(' list-group-item-action');
    //     savedStationsList.append(newChild);
    // })
}

function executeLoop() {
    // fetchStations();
    addStationsToDOM();
    initIsFavourite();
    updateCurrentStation(currentStation);
}

executeLoop()