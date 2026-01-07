const button_profile = document.querySelector(".profile");
const button_setting = document.querySelector(".setting_icon");
const button_mode = document.querySelector(".mode");
const img_mode = document.querySelector(".img_mode");
const wifi = document.getElementById("wifi");
const gsm = document.getElementById("gsm");
const user = document.getElementById("user");

function theme() {
    // document.body.style.backgroundColor = this.isOn ? "#F8FAFC" :"#222831" ;
    document.documentElement.classList.remove("light", "dark");
    document.documentElement.classList.add(this.isOn ? "light" :"dark");
    img_mode.src = this.isOn ?  "icon/moon.png":"icon/sun.png" ;
    localStorage.setItem("dark_mode",this.isOn ? "true" : "false");

}


const mode = new toggle(true, theme, button_mode);
mode.click();
const last_mode  = localStorage.getItem("dark_mode") === "true";
mode.reload(last_mode);

const buttons = [wifi,gsm, user];
const menu = ["wifi_menu","gsm_menu","user_menu"];

function activate(activeBtn,active_menu) {
    for(let x in buttons){
    buttons[x].style.backgroundColor = "black";
    }
    for(let i in menu){
    document.getElementById(menu[i]).style.visibility = "hidden";
    document.getElementById(menu[i]).style.display = "none";
}
document.getElementById(active_menu).style.visibility = "visible";
document.getElementById(active_menu).style.display = "flex";

activeBtn.style.backgroundColor = "red";
}
activate(wifi,"wifi_menu");
button("wifi", () => {activate(wifi,"wifi_menu")});
button("gsm",  () => {activate(gsm,"gsm_menu")});
button("user",  () => {activate(user,"user_menu")});

function click_setting(){
    document.querySelector(".setting_p").style.display = this.isOn ? "flex" : "none";
}
const setting = new toggle(true,click_setting,button_setting);
setting.click();
setting.reload();

function click_profile(){
    document.querySelector(".profile_p").style.display = this.isOn ? "flex" : "none";
}
const profile = new toggle(true,click_profile,button_profile);
profile.click();
profile.reload();

function updateUI(data) {
        document.getElementById("level_oil").innerText = data.level_oil;
        document.getElementById("temp_oil").innerText = data.temp_oil;
        document.getElementById("temp_air").innerText = data.temp_air;
        document.getElementById("voltage_battery").innerText = data.voltage_battery;
        document.getElementById("runtime_value").innerHTML = dtat.runtime; 
}

const socket = new WebSocket("ws://192.168.4.1/ws");


socket.addEventListener("open", () => {
    console.log("Connected to ESP32 WebSocket");
});

const set_wifi_name = button("set_wifi_name",()=>{
    socket.send(JSON.stringify({wifi_name : document.getElementById("wifi_name").value }));
    // alert(JSON.stringify({wifi_name : document.getElementById("wifi_name").value}));
   
});

const set_wifi_password = button("set_wifi_password",()=>{
    socket.send(JSON.stringify({wifi_password : document.getElementById("wifi_password").value }));
    // alert(JSON.stringify({wifi_password : document.getElementById("wifi_password").value }));
});

const set_gps_keyword = button("set_gps_keyword", ()=>{
    socket.send(JSON.stringify({gps_keyword : document.getElementById("gps_keyword").value}));
    //  alert(JSON.stringify({gps_keyword : document.getElementById("gps_keyword").value}));
});

const set_gps_password = button("set_gps_password",()=>{
    socket.send(JSON.stringify({gps_password : document.getElementById("gps_password").value}));
    //  alert(JSON.stringify({gps_password : document.getElementById("gps_password").value}));
});

const message_test = button("message_test" , ()=>{socket.send("message_test");});
const test_call = button("call_test" , ()=>{socket.send("call_test");}); 
const reset_runtime = button("reset_runtime" , ()=>{socket.send("reset_runtime");});

    document.getElementById("message_test").addEventListener("click", ()=>{socket.send("message_test");});


socket.addEventListener("message", (event) => {
    console.log("Data received from ESP32:", event.data);
    try {
        const json = JSON.parse(event.data);
        updateUI(json);
    } catch (e) {
        console.error("Invalid JSON", e);
    }
});


socket.addEventListener("close", () => {
    console.log("WebSocket closed");
});

