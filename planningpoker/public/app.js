// Firebase modular SDK imports
import {
    initializeApp
}
from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";
import {
    getFirestore,
    doc,
    getDoc,
    getDocs,
    setDoc,
    deleteDoc,
    collection,
    onSnapshot,
    updateDoc,
    serverTimestamp
}
from "https://www.gstatic.com/firebasejs/10.12.2/firebase-firestore.js";
import {
    getAuth,
    onAuthStateChanged,
    signInAnonymously
}
from "https://www.gstatic.com/firebasejs/10.12.2/firebase-auth.js";

// Firebase config
const firebaseConfig = {
    apiKey: "AIzaSyCWZnCICuraksaBEbqdOrxCBkp4eG_jvRM",
    authDomain: "planningpoker-18fcb.firebaseapp.com",
    projectId: "planningpoker-18fcb",
    storageBucket: "planningpoker-18fcb.firebasestorage.app",
    messagingSenderId: "274728253329",
    appId: "1:274728253329:web:7ff8a0a9b36a19d0e78c30"
};

const app = initializeApp(firebaseConfig);
const db = getFirestore(app);
const auth = getAuth(app);
let OpenOptionOnce = false;

// Sign in anonymously
signInAnonymously(auth).catch(console.error);

let currentUser = null;
onAuthStateChanged(auth, (user) => {
    if (user)
        currentUser = user;
});

// Utils
function randomRoomId() {
    return Math.random().toString(36).substring(2, 8).toUpperCase();
}

async function createRoom() {
    const name = document.getElementById("voterName").value.trim();
    const cardType = document.getElementById("cardType").value;
    if (!name)
        return alert("Enter your name");
    localStorage.setItem("voterName", name);
    document.getElementById("LoginOrCreateForm").style.display = "none";
    OpenOptionOnce = true;

    const roomId = randomRoomId();
    const roomRef = doc(db, "rooms", roomId);

    await setDoc(roomRef, {
        name: roomId,
        owner: currentUser.uid,
        state: "voting",
        topic: "",
        cardType: cardType,
        createdAt: serverTimestamp()
    });

    await joinRoom(roomId, name, true);

    window.history.pushState({}, "", `?room=${roomId}`);
}

async function joinRoom(roomIdInput = null, nameInput = null, isOwner = false) {
    const name = nameInput || document.getElementById("voterName").value.trim();
    const roomId = roomIdInput || document.getElementById("roomIdInput").value.trim().toUpperCase();
    if (!name || !roomId)
        return alert("Enter both name and room ID");
    localStorage.setItem("voterName", name);

    const participantRef = doc(db, "rooms", roomId, "participants", currentUser.uid);
    const savedHideCard = localStorage.getItem("hideCard") || false;

    await setDoc(participantRef, {
        name,
        card: null,
        hasVoted: false,
        hideCard: savedHideCard,
        lastActive: serverTimestamp()
    });

    watchRoom(roomId);
}

function watchRoom(roomId) {
    //console.log(`Watching room ${roomId}`);
    const roomRef = doc(db, "rooms", roomId);
    const participantsRef = collection(db, "rooms", roomId, "participants");

    let latestRoom = null;
    let latestParticipants = [];

    onSnapshot(roomRef, (docSnap) => {
        if (!docSnap.exists()) {
            alert("Room does not exist or has been deleted.");
            document.getElementById("LoginOrCreateForm").style.display = "block";
            document.getElementById("room").innerHTML = "";
            document.getElementById("roomIdInput").value = "";
            window.history.pushState({}, "", window.location.origin); // reset URL
            return;
        }
        latestRoom = docSnap.data();
        tryRender();
    });

    onSnapshot(participantsRef, (snapshot) => {
        latestParticipants = snapshot.docs.map(doc => ({
                    id: doc.id,
                    ...doc.data()
                }));
        tryRender();
    });

    function tryRender() {
        if (latestRoom && currentUser) {
            const me = latestParticipants.find(p => p.id === currentUser.uid) || {};
            renderRoom(latestRoom, me, latestParticipants);
        }
    }
}

window.submitVote = async function submitVote(roomId, cardValue) {
    //console.log(`roomId = ${roomId} submitting card: ${cardValue}`);
    const participantRef = doc(db, "rooms", roomId, "participants", currentUser.uid);

    await updateDoc(participantRef, {
        card: cardValue,
        hasVoted: true,
        lastActive: serverTimestamp()
    });
}

// Ensure global access for buttons
window.handleCreateRoom = async function () {
    if (!currentUser) {
        await new Promise(resolve => {
            const unsub = onAuthStateChanged(auth, user => {
                if (user) {
                    currentUser = user;
                    unsub();
                    resolve();
                }
            });
        });
    }

    createRoom();
};

window.handleJoinRoom = async function () {
    if (!currentUser) {
        await new Promise(resolve => {
            const unsub = onAuthStateChanged(auth, user => {
                if (user) {
                    currentUser = user;
                    unsub();
                    resolve();
                }
            });
        });
    }

    joinRoom();
};

window.addEventListener("DOMContentLoaded", () => {
    cleanupNow();
    const urlParams = new URLSearchParams(window.location.search);
    const roomIdFromUrl = urlParams.get("room");
    if (roomIdFromUrl) {
        document.getElementById("roomIdInput").value = roomIdFromUrl;
    }

    const savedName = localStorage.getItem("voterName");
    if (savedName) {
        document.getElementById("voterName").value = savedName;
    }

    if (roomIdFromUrl && savedName) {
        document.getElementById("LoginOrCreateForm").style.display = "none";
        const tryAutoJoin = () => {
            if (currentUser) {
                //console.log(`Auto-joining room ${roomIdFromUrl} as ${savedName}`);
                joinRoom(roomIdFromUrl, savedName);
            } else {
                onAuthStateChanged(auth, user => {
                    if (user) {
                        currentUser = user;
                        //console.log(`Auto-joining room ${roomIdFromUrl} as ${savedName} (after auth ready)`);
                        joinRoom(roomIdFromUrl, savedName);
                    }
                });
            }
        };
        tryAutoJoin();
    }
});

window.updateTopic = async function (roomId) {
    const input = document.getElementById("topicInput");
    const topic = input.value.trim();
    const roomRef = doc(db, "rooms", roomId);
    await updateDoc(roomRef, {
        topic: topic
    });
};

window.startVote = async function (roomId) {
    const participantsRef = collection(db, "rooms", roomId, "participants");
    const snapshot = await getDocs(participantsRef);

    // Reset participants first
    const updates = snapshot.docs.map(docSnap =>
            updateDoc(docSnap.ref, {
                card: null,
                hasVoted: false
            }));

    await Promise.all(updates);

    // Then set room state
    const roomRef = doc(db, "rooms", roomId);
    await updateDoc(roomRef, {
        state: "voting"
    });
};

window.revealVotes = async function (roomId) {
    const roomRef = doc(db, "rooms", roomId);
    await updateDoc(roomRef, {
        state: "voted"
    });
};

window.updateHideCard = async function (roomId) {
    const checkbox = document.getElementById("hideCardCheckbox");
    localStorage.setItem("hideCard", checkbox.checked ? "true" : "false");
    const participantRef = doc(db, "rooms", roomId, "participants", currentUser.uid);
    await updateDoc(participantRef, {
        hideCard: checkbox.checked
    });
};

function formatTopic(topic) {
    const urlPattern = /^(https?:\/\/[^\s]+)$/i;
    if (urlPattern.test(topic)) {
        return `<a href="${topic}" target="_blank" rel="noopener noreferrer">${topic}</a>`;
    }
    return topic || "No topic set";
}

async function renderRoom(room, myParticipant, participants = []) {
    const roomId = room.name;
    const roomDiv = document.getElementById("room");
    const topicDiv = document.getElementById("topicFrame");
    const roomOptionsDiv = document.getElementById("roomOptionsFrame");

    //console.log("Rendering room:", room.name);
    //console.log("Room owner:", room.owner);
    //console.log("Current user:", currentUser?.uid);

    if (topicDiv){
		if(room.topic.length > 0)
			topicDiv.innerHTML = `<p><strong>Topic:</strong> ${formatTopic(room.topic)}</p>`;
		else
			topicDiv.innerHTML = "";
	}

    let topicHTML = "";
    if (room.owner === currentUser.uid)
        topicHTML += `<input id="topicInput" value="${room.topic || ""}" placeholder="Set topic..." />
						<button onclick="updateTopic('${roomId}')">Update</button>`;

    let voteButtons = "";
    if (room.state === "voting" && room.cardType) {
        let cardOptionsValues = [];
        let cardOptionsNames = [];
        if (room.cardType == "numeric") {
            cardOptionsValues = [0, 1, 2, 3, 4, 5, 6];
            cardOptionsNames = ["Pass", "1", "2", "3", "5", "8", "13"];
        } else if (room.cardType == "tshirt") {
            cardOptionsValues = [0, 1, 2, 3, 4, 5, 6];
            cardOptionsNames = ["Pass", "XS", "S", "M", "L", "XL", "XXL"];
        }
        voteButtons = `<div id="cardButtons">Pick your card:`;
        cardOptionsValues.forEach(card => {
            const hideCard = String(myParticipant.hideCard) === "true";
            const isSelected = Number(myParticipant.card) === Number(card) && !hideCard && Number(card) != 0;

            /*console.log("Rendering card button:", {
            card,
            selected: myParticipant.card,
            isSelected,
            hideCard: myParticipant.hideCard
            });*/

            const style = isSelected ? 'background-color: #4CAF50;' : "";
            voteButtons += `<button style="width: 50px; border-radius: 0; box-shadow: none; border: none; ${style}" onclick="submitVote('${room.name}', ${card})">` + cardOptionsNames[card] + `</button>`;
        });
        voteButtons += `</div>`;
    }

    let ownerControls = "";
    if (room.owner === currentUser.uid) {
        ownerControls = `
      <div id="ownerControls">
        <button onclick="startVote('${roomId}')">Start New Vote</button>
        <button onclick="revealVotes('${roomId}')">Reveal Votes</button>
      </div>
    `;
    }

    const link = `${window.location.origin}?room=${roomId}`;
    let savedHideCard = localStorage.getItem("hideCard") === "true" ? "checked" : "";
    let hideCardOption = `
    <label>
      <input type="checkbox" id="hideCardCheckbox" onchange="updateHideCard('${roomId}')" ${savedHideCard}>
      Hide my card value until reveal <span title="Useful during screen sharing">🛈</span>
    </label>
  `;

    let changeNameHTML = `
	  <div id="changeName">
		<button onclick="changeName('${room.name}')">Change name</button><input type="text" id="changeNameInput" value="${myParticipant.name || ""}" />
	  </div>
	`;

    let linkToJoin = `
	  <button onclick="copyToClipboard('${link}')">📋 Click to copy join link</button>
	  <span style="margin-left: 1em;"><a href="${window.location.origin}">Reset page</a></span>
	`;

    if (roomOptionsDiv) {
        roomOptionsDiv.innerHTML = `
		  ${linkToJoin} <br/>
		  ${changeNameHTML}
		  ${topicHTML} <br/>
		  ${hideCardOption}
		`;
    }

    let roomHTML = `
    ${ownerControls}
    ${voteButtons}
  `;

    //console.log("Loaded participants:", participants);

    let cardOptionsNames = [];
    if (room.cardType == "numeric") {
        cardOptionsNames = ["Pass", "1", "2", "3", "5", "8", "13"];
    } else if (room.cardType == "tshirt") {
        cardOptionsNames = ["Pass", "XS", "S", "M", "L", "XL", "XXL"];
    }

    // Participant list
    let participantListHTML = "<h3>Participants</h3>";

    // Calculate grid size (ceil of square root)
    const gridSize = Math.ceil(Math.sqrt(participants.length));

    participantListHTML += `<div style="
	  display: grid;
	  grid-template-columns: repeat(${gridSize}, 1fr);
	  gap: 10px;
	  max-width: 300px;
	">`;

    participants.forEach(p => {
        const voted = p.hasVoted ? "✅" : "❌";
        const isSelf = p.id === currentUser.uid ? " (You)" : "";
        const cardDisplay = room.state === "voted" && p.card !== null
             ? `<div><strong>${cardOptionsNames[p.card]}</strong></div>`
             : "";

        participantListHTML += `
		<div style="border: 1px solid #ccc; padding: 8px; text-align: center; border-radius: 5px;">
		  <div>${p.name}${isSelf}</div>
		  <div>${voted}</div>
		  ${cardDisplay}
		</div>`;
    });

    participantListHTML += "</div>";

    // If voted state, show reveal summary
    let revealHTML = "";
    if (room.state === "voted") {
        const visibleVotes = participants
            .filter(p => typeof p.card === "number")
            .map(p => ({
                    name: p.name,
                    card: p.card
                }));

        if (visibleVotes.length > 0) {
            const values = visibleVotes.map(p => p.card).filter(val => val !== 0);
            if (values.length > 0) {
                const min = Math.min(...values);
                const max = Math.max(...values);
                let avg = (values.reduce((a, b) => a + b, 0) / values.length).toFixed(2);

                const minVoters = visibleVotes.filter(p => p.card === min).map(p => p.name).join(", ");
                const maxVoters = visibleVotes.filter(p => p.card === max).map(p => p.name).join(", ");

                let cardOptionsNames = [];
                if (room.cardType == "numeric") {
                    cardOptionsNames = ["Pass", "1", "2", "3", "5", "8", "13"];
                    avg = Math.round(avg);
                } else if (room.cardType == "tshirt") {
                    cardOptionsNames = ["Pass", "XS", "S", "M", "L", "XL", "XXL"];
                    avg = Math.round(avg);
                }

                let breakdown = "";
                const grouped = {};
                visibleVotes.forEach(p => {
                    grouped[p.card] = grouped[p.card] || [];
                    grouped[p.card].push(p.name);
                });
                for (const val in grouped) {
                    breakdown += `<li>${cardOptionsNames[val]}: ${grouped[val].join(", ")}</li>`;
                }

                revealHTML = `
			  <h3>Vote Summary</h3>
			  <p>Min: ${cardOptionsNames[min]} (${minVoters})</p>
			  <p>Max: ${cardOptionsNames[max]} (${maxVoters})</p>
			  <p>Average: ${cardOptionsNames[avg]}</p>
			  <ul>${breakdown}</ul>
			`;
            }
        }
    }

    roomHTML += participantListHTML + revealHTML;
    roomDiv.innerHTML = roomHTML;

    if (OpenOptionOnce == true && room.owner === currentUser.uid) {
        const panel = document.getElementById("roomOptionsPanel");
        if (panel)
            panel.setAttribute("open", "true");
        OpenOptionOnce = false;
    }
}

window.changeName = async function changeName(roomId) {
    const newName = document.getElementById("changeNameInput").value.trim();
    if (!newName)
        return alert("Name can't be empty");

    const participantRef = doc(db, "rooms", roomId, "participants", currentUser.uid);
    await updateDoc(participantRef, {
        name: newName
    });
    localStorage.setItem("voterName", newName);
    //console.log(`Name updated to ${newName}`);
};

// WARNING: For internal use only!
async function cleanupNow(maxMinutes = 3 * 60) {
    const roomsSnap = await getDocs(collection(db, "rooms"));
    const now = Date.now();

    for (const roomDoc of roomsSnap.docs) {
        const roomId = roomDoc.id;
        const participantsRef = collection(db, "rooms", roomId, "participants");
        const participantsSnap = await getDocs(participantsRef);

        const recent = participantsSnap.docs.find(doc => {
            const last = doc.data().lastActive?.toDate();
            return last && (now - last.getTime() < maxMinutes * 60 * 1000);
        });

        if (!recent) {
            console.log(`Deleting inactive room ${roomId}`);
            await Promise.all(participantsSnap.docs.map(doc => deleteDoc(doc.ref)));
            await deleteDoc(doc(db, "rooms", roomId));
        }
    }
};

window.showToast = function (message = "Copied!") {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.style.display = "block";
    setTimeout(() => {
        toast.style.display = "none";
    }, 2000); // disappears after 2 seconds
};

window.copyToClipboard = function (text) {
    navigator.clipboard.writeText(text)
    .then(() => showToast("Link copied to clipboard!"))
    .catch(err => showToast("Failed to copy 😞"));
};