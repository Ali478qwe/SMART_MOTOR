  class toggle{
   isOn;
   button;
   
  constructor(isOn=false,func_style,button) {
    this.isOn = isOn;
    this.button = button;
   this.func_style = func_style.bind(this);
    
    
   
  }
  click() {
      this.button.addEventListener("click", () => {
      this.isOn = !this.isOn;
       this.func_style();
    });
  }
  reload(isOn) {
     this.isOn = isOn;
     this.func_style();
  }
}

function button(id,func){
      document.getElementById(id).addEventListener("click",func);
}
