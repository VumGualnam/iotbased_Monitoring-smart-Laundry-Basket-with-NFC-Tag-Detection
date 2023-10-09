function openNav(withAnim = true) {
  document.getElementsByClassName("sidenav")[0].style.width = "250px";
  localStorage.setItem("navState", "open");
}

function closeNav() {
  document.getElementsByClassName("sidenav")[0].style.width = "0";
  localStorage.setItem("navState", "closed");
}

document.addEventListener("DOMContentLoaded", (event) => {
  var navState = localStorage.getItem("navState");
  if (navState === "open") {
    openNav();
  }
});
