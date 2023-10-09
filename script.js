function moveTriangleByData(dataValue) {
  const triangle = document.getElementById("triangle");
  let positions = [0, 50, 100]; // pixel as unit

  let positionIndex; //change if needed
  if (dataValue <= 33) {
    positionIndex = 0;
  } else if (dataValue <= 66) {
    positionIndex = 1;
  } else {
    positionIndex = 2;
  }

  triangle.style.bottom = positions[positionIndex] + "px";
}

function BasketFillByData(dataValue) {
  const basketFill = document.getElementById("basketFill");
  let heightP;
  if (dataValue <= 0) {
    heightP = "0%";
  } else if (dataValue <= 4) {
    heightP = "4%";
  } else if (dataValue <= 8) {
    heightP = "8%";
  } else if (dataValue <= 12) {
    heightP = "12%";
  } else if (dataValue <= 16) {
    heightP = "16%";
  } else {
    heightP = "20%";
  }
  basketFill.style.height = heightP;
}

function fetchCapacity() {
  fetch("/getCapacity") //get rfid count by function in esp32 hard coding
    .then((response) => response.json())
    .then((data) => {
      BasketFillByData(data.capacity);
    })
    .catch((error) => console.error("Error fetching capacity", error));
}

setInterval(fetchCapacity, 5000);

function Notification(message) {
  const messageBox = document.getAnimations("messageBox");

  messageBox.innerHTML = message;
}
