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

async function fetchStations() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            savedStationsArr = JSON.parse(this.responseText).stations;
            currentStation = savedStationsArr[0];
            favouritesStationsArr = savedStationsArr.filter(s => s.isFavourite);
            initIsFavourite();
        }
    }
    xmlhttp.open("GET", "/stations", true);
    xmlhttp.send()
}

async function initIsFavourite() {
    console.log(currentStation);
    (currentStation.isFavourite) ? $('#add-favourite-heart').addClass('d-none') : $('#delete-favourite-heart').addClass('d-none');
}

async function checkIsFavourite(prevStation, newStation) {
    if (prevStation.isFavourite !== newStation.isFavourite) {
        if (prevStation.isFavourite) {
            $('#add-favourite-heart').removeClass('d-none');
            $('#delete-favourite-heart').addClass('d-none')
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
    xmlhttp.send('id=' + newStation.id)
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
            if (currentStation === station) currentStation = savedStationsArr[0];
            updateStationsDOM();
        }
    }

    xmlhttp.open("POST", "/stations", true);
    xmlhttp.send('id=' + station.id)
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
            updateStationsDOM();
        }
    }
    xmlhttp.open(currentStation.isFavourite ? "DELETE" : "POST", "/favourites", true);
    xmlhttp.send('id=' + currentStation.id);

}

async function selectPreviousStation() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStationDOM(this.responseText)
        }
    }

    let nextStationIndex = savedStationsArr.findIndex((s) => s.id === currentStation.id) - 1;
    if (nextStationIndex < 0) nextStationIndex = savedStationsArr.length - 1;
    const nextStation = savedStationsArr[nextStationIndex];

    xmlhttp.open("POST", "/set-station", true);
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
    xmlhttp.open("GET", "/toggle-play", true);
    xmlhttp.send();
}

function selectNextStation() {
    xmlhttp.onreadystatechange = function () {
        if (this.readyState === 4 && this.status === 200) {
            updateCurrentStationDOM(this.responseText)
        }
    }
    let nextStationIndex = savedStationsArr.findIndex((s) => s.id === currentStation.id) + 1;
    if (nextStationIndex >= savedStationsArr.length) nextStationIndex = 0;
    const nextStation = savedStationsArr[nextStationIndex];
    xmlhttp.open("POST", "/set-station", true);
    xmlhttp.send('id=' + nextStation.id)
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
            updateStationsDOM();
        }
    }
    const formData = new FormData();
    formData.append('name', $('#name').val());
    formData.append('host', $('#host').val());
    formData.append('port', $('#port').val());
    formData.append('path', $('#path').val());

    xmlhttp.open("POST", "/stations", true);
    xmlhttp.send(formData);
}


function addStationsToDOM() {
    savedStationsArr.forEach((s) => {
        const newChild = document.createElement('a');
        const deleteButton = document.createElement('button');

        $(deleteButton).html('delete').addClass('btn btn-danger').click(() => deleteStation(s))

        $(newChild).html(s.name).addClass('list-group-item')
            .addClass(' list-group-item-action').click(() => updateCurrentStation(s)).append(deleteButton)
        document.getElementById('#saved-stations').appendChild(newChild);
    })
}

function addFavouriteStationsToDOM() {
    favouritesStationsArr = savedStationsArr.filter(s => s.isFavourite);

    favouritesStationsArr.forEach((s) => {
        const newChild = document.createElement('a');
        const deleteButton = document.createElement('button');

        $(deleteButton).html('delete').addClass('btn btn-danger').click(() => deleteStation(s))

        $(newChild).html(s.name).addClass('list-group-item')
            .addClass('list-group-item-action').click(() => updateCurrentStation(s)).append(deleteButton);

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

    initStationsDOM();
}

async function executeLoop() {
    await fetchStations().then(() => {
        initStationsDOM()
        initIsFavourite();
        updateCurrentStationDOM(currentStation);
    });
}

executeLoop()