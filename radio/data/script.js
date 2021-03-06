let favouritesStationsArr = [];
let savedStationsArr = [
    // {
    //     id: 1, isFavourite: false, name: 'hej',
    // },
    // {
    //     id: 32, isFavourite: true, name: 'Andrzejska'
    // },
    // {
    //     id: 31, isFavourite: false, name: 'Mother'
    // },
    // {
    //     id: 30, isFavourite: true, name: 'Father'
    // },
    // {
    //     id: 29, isFavourite: false, name: 'Sister'
    // },
    // {
    //     id: 28, isFavourite: true, name: 'Pop'
    // },
    // {
    //     id: 27, isFavourite: false, name: 'Lo-fi'
    // },
    // {
    //     id: 26, isFavourite: true, name: 'Rock'
    // },
    // {
    //     id: 25, isFavourite: true, name: 'Drum and Bass'
    // }
];
let currentStation = savedStationsArr[0];
let isPlaying = false;
let xmlhttp = new XMLHttpRequest();

var gateway = `ws://${window.location.hostname}/update`;
var websocket;
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    data = JSON.parse(event.data)
    updateCurrentStationDOM(data.station);
    isPlaying = data.playing;
    checkIsPlaying();
}

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

async function fetchStations() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr = JSON.parse(this.responseText).stations;
            currentStation = savedStationsArr[0];
            favouritesStationsArr = savedStationsArr.filter(s => s.isFavourite);
            initIsFavourite();
            updateStationsDOM();
            updateCurrentStationDOM(currentStation);
            checkStationState();
        }
    }
    xmlhttp.open("GET", "/stations", true);
    xmlhttp.send()
}

async function initIsFavourite() {
    console.log(currentStation);
    (currentStation.isFavourite) ? $('#add-favourite-heart').addClass('d-none') : $('#delete-favourite-heart').addClass('d-none');
}

function checkIsFavourite(prevStation, newStation) {
    if (prevStation.isFavourite !== newStation.isFavourite) {
        if (prevStation.isFavourite) {
            $('#add-favourite-heart').removeClass('d-none');
            $('#delete-favourite-heart').addClass('d-none');
        } else {
            $('#add-favourite-heart').addClass('d-none');
            $('#delete-favourite-heart').removeClass('d-none')

        }
    }
}

async function updateCurrentStation(newStation) {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStationDOM(newStation)
        }
    }

    xmlhttp.open("POST", "/set-station", true);
    const data = JSON.stringify({id: newStation.id})
    xmlhttp.send(data)
}

async function updateCurrentStationDOM(newStation) {
    checkIsFavourite(currentStation, newStation)
    currentStation = newStation;
    $('#current-station').html(currentStation.name);
}

async function deleteStation(station) {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr = savedStationsArr.filter(s => s.id !== station.id);
            favouritesStationsArr = savedStationsArr.filter(s => s.isFavourite);
            if (currentStation === station) {
                currentStation = savedStationsArr[0];
                updateCurrentStation(currentStation);
            }
            updateStationsDOM();
        }
    }

    xmlhttp.open("DELETE", "/stations", true);
    const data = JSON.stringify({id: station.id})
    xmlhttp.send(data)
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
            console.log("before update");
            fetchStations();
        }
    }
    xmlhttp.open(currentStation.isFavourite ? "DELETE" : "POST", "/favourites", true);
    const data = JSON.stringify({id: currentStation.id})
    xmlhttp.send(data)

}

async function selectPreviousStation() {
    let nextStationIndex = savedStationsArr.findIndex((s) => s.id === currentStation.id) - 1;
    if (nextStationIndex < 0) nextStationIndex = savedStationsArr.length - 1;
    const nextStation = savedStationsArr[nextStationIndex];

    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStationDOM(nextStation)
        }
    }

    xmlhttp.open("POST", "/set-station", true);
    const data = JSON.stringify({id: nextStation.id})
    xmlhttp.send(data)
}

function togglePlayState() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            isPlaying = !isPlaying;
            checkIsPlaying();
        }
    }
    xmlhttp.open("GET", "/toggle-play", true);
    xmlhttp.send();
}

function checkIsPlaying() {
    const playButton = $('#play-button');
    playButton.html(isPlaying ? 'Pause' : 'Play');
    playButton.removeClass(isPlaying ? 'btn-success' : 'btn-danger');
    playButton.addClass(isPlaying ? 'btn-danger' : 'btn-success');
}

async function checkStationState() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            const response = JSON.parse(this.responseText);
            updateCurrentStationDOM(response.station);
            isPlaying = response.playing;
            checkIsPlaying();
        }
    }
    xmlhttp.open("GET", "/check-station", true);
    xmlhttp.send();
}

function selectNextStation() {
    let nextStationIndex = savedStationsArr.findIndex((s) => s.id === currentStation.id) - 1;
    if (nextStationIndex < 0) nextStationIndex = savedStationsArr.length - 1;
    const nextStation = savedStationsArr[nextStationIndex];

    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStationDOM(nextStation)
        }
    }

    xmlhttp.open("POST", "/set-station", true);
    const data = JSON.stringify({id: nextStation.id})
    xmlhttp.send(data)
}


function makeMoreQuiet() {
    xmlhttp.onreadystatechange = () => {
    };
    xmlhttp.open("GET", "/more-quiet", true)
    xmlhttp.send();
}

function makeLouder() {
    xmlhttp.onreadystatechange = () => {
    };
    xmlhttp.open("GET", "/louder", true)
    xmlhttp.send();
}

function addStationToSaved() {
    const formData = JSON.stringify({
        'name': $('#name').val(),
        'host': $('#host').val(),
        'port': $('#port').val(),
        'path': $('#path').val()
    });

    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr.push(JSON.parse(formData));
            $('#name').html('');
            $('#host').html('');
            $('#port').html('');
            $('#path').html('');

            $(document).ready(function () {
                $("#liveToast").toast('show');
            })
            updateStationsDOM();
        }
    }

    xmlhttp.open("POST", "/stations", true);
    xmlhttp.send(formData);
}


function addStationsToDOM() {
    savedStationsArr.forEach((s) => {
        const newDiv = document.createElement('div')
        const newChild = document.createElement('a');
        const deleteButton = document.createElement('button');

        $(deleteButton).html('delete').addClass('btn btn-danger').click(() => deleteStation(s))

        $(newChild).addClass('list-group-item')
            .addClass(' list-group-item-action')

        $(newDiv).html(s.name).click(() => updateCurrentStation(s));

        $(newChild).append(newDiv).append(deleteButton)

        document.getElementById('#saved-stations').appendChild(newChild);
    })
}

function addFavouriteStationsToDOM() {
    favouritesStationsArr = savedStationsArr.filter(s => s.isFavourite);
    console.log(currentStation);
    console.log(savedStationsArr);

    favouritesStationsArr.forEach((s) => {
        const newDiv = document.createElement('div')
        const newChild = document.createElement('a');
        const deleteButton = document.createElement('button');

        $(deleteButton).html('delete').addClass('btn btn-danger').click(() => deleteStation(s))

        $(newChild).addClass('list-group-item')
            .addClass(' list-group-item-action')

        $(newDiv).html(s.name).click(() => updateCurrentStation(s));

        $(newChild).append(newDiv).append(deleteButton)

        document.getElementById('#favourites-stations').appendChild(newChild);
    });
}

function initStationsDOM() {
    addStationsToDOM();
    addFavouriteStationsToDOM();
}

function updateStationsDOM() {
    const favouritesStationsDOM = document.getElementById('#favourites-stations');
    $(favouritesStationsDOM).empty();

    const savedStationsDOM = document.getElementById('#saved-stations');
    $(savedStationsDOM).empty();

    console.log("emptied");
    initStationsDOM();
}

async function executeLoop() {
    await fetchStations();
    // setInterval(() => checkStationState(), 1000);
}

executeLoop()