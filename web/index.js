{
        const url = "data.json";    // 読み込むJSONファイル
 
    function formatJSON(json){
        console.log(json);
 
        let html = "";
	let i=0;
        for(let data of json){
	    let first = document.createElement('div');
	    first.textContent = i;
	    first.classList.add("table__item");
	    let name = document.createElement('div');
	    name.textContent = data.name;
	    name.classList.add("table__item")
	    let number = document.createElement('div');
	    number.textContent = data.number;
	    number.classList.add("table__item")
	    let age = document.createElement('div');
	    age.textContent = data.age;
	    age.classList.add("table__item");
	    
	    let table = document.querySelector(".table");
	    table.appendChild(first);
	    table.appendChild(name);
	    table.appendChild(number);
	    table.appendChild(age);
	    i++;
        }
        
    }
 
// 起動時の処理
    window.addEventListener("load", ()=>{
        fetch(url)
            .then( response => response.json())
            .then( data => formatJSON(data));
    });
  
}
