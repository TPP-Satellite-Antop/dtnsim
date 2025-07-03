#include <src/node/dtn/ContactPlan.h>

ContactPlan::~ContactPlan()
{

}

ContactPlan::ContactPlan()
{

}

ContactPlan::ContactPlan(ContactPlan &contactPlan)
{
	this->nextContactId = contactPlan.nextContactId;
	this->lastEditTime = contactPlan.lastEditTime;
	this->contactsFile_ = contactPlan.contactsFile_;
	this->contacts_ = contactPlan.contacts_;
	this->ranges_ = contactPlan.ranges_;
	this->contactIdShift_ = contactPlan.contactIdShift_;
	this->contactIdsBySrc_ = contactPlan.contactIdsBySrc_;

	for (size_t i = 0; i < contacts_.size(); i++)
	{
		contacts_.at(i).work = NULL;
	}

	for (size_t i = 0; i < ranges_.size(); i++)
	{
		ranges_.at(i).work = NULL;
	}
}

/**
 * Sets up the contactPlan for the simulations
 *
 * @param fileName: The name of the contact plan .txt file
 * 		  nodesNumber: The number of nodes
 * 		  mode: The chosen mode for the simulation (0, 1, 2)
 * 		  failureProb: The failure probability every contact should have, -1 if the probability from the contact plan file should be taken
 *
 * @authors: original implementation from the authors of DTNSim, then modified by Simon Rink
 */
void ContactPlan::parseContactPlanFile(string fileName, int nodesNumber, int mode, double failureProb)
{
	this->contactIdsBySrc_.resize(nodesNumber + 1);

	double start = 0.0;
	double end = 0.0;
	int sourceEid = 0;
	int destinationEid = 0;
	double dataRateOrRange = 0.0;
	double failureProbability = 0.0;

	string fileLine = "#";
	string a;
	string command;
	ifstream file;

	file.open(fileName.c_str());

	if (!file.is_open())
		throw cException("%s", ("Error: wrong path to contacts file " + fileName).c_str());

	while (getline(file, fileLine))
	{
		if (fileLine.empty())
			continue;

		if (fileLine.at(0) == '#')
			continue;

		// This seems to be a valid command line, parse it
		stringstream stringLine(fileLine);
		stringLine >> a >> command >> start >> end >> sourceEid >> destinationEid >> dataRateOrRange >> failureProbability;

		if (failureProb >= 0)
		{
			failureProbability = failureProb;
		}

		if (a.compare("a") == 0)
		{
			if ((command.compare("contact") == 0))
			{
				this->addContact(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0, (double) failureProbability / 100);
			}
			else if ((command.compare("range") == 0))
			{
				this->addRange(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0);
			}
			else if ((command.compare("ocontact") == 0))
			{
				if (mode < 2)
				{
					continue;
				}
				else
				{
					this->addContact(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0, 0);
					this->addContact(start, end, destinationEid, sourceEid, dataRateOrRange, (double) 1.0, 0);
				}
			}
			else if ((command.compare("orange") == 0))
			{
				if (mode < 2)
				{
				continue;
				}
				else
				{
					this->addRange(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0);
					this->addRange(start, end, destinationEid, sourceEid, dataRateOrRange, (double) 1.0);
				}
			}
			else
			{
				cout << "dtnsim error: unknown contact plan command type: a " << fileLine << endl;
			}
		}
	}

	if (cin.bad())
	{
		// IO error
	}
	else if (!cin.eof())
	{
		// format error (not possible with getline but possible with operator>>)
	}
	else
	{
		// format error (not possible with getline but possible with operator>>)
		// or end of file (can't make the difference)
	}

	file.close();

	this->setContactsFile(fileName);
	this->updateContactRanges();
	this->sortContactIdsBySrcByStartTime();
}

/**
 * Sets up the contactTopology for the simulations
 *
 * @param fileName: The name of the contact plan .txt file
 * 		  nodesNumber: The number of nodes
 * 		  mode: The chosen mode for the simulation (0, 1, 2)
 * 		  failureProb: The failure probability every contact should have, -1 if the probability from the contact plan file should be taken
 *
 * @authors: adapted from parseContactPlanFile() from the authors of DTNSim, then ported and modified by Simon Rink
 */
void ContactPlan::parseOpportunisticContactPlanFile(string fileName, int nodesNumber, int mode, double failureProb)
{
	this->contactIdsBySrc_.resize(nodesNumber + 1);

	double start = 0.0;
	double end = 0.0;
	int sourceEid = 0;
	int destinationEid = 0;
	double dataRateOrRange = 0.0;
	double failureProbability = 0;

	string fileLine = "#";
	string a;
	string command;
	ifstream file;

	file.open(fileName.c_str());

	if (!file.is_open())
		throw cException("%s", ("Error: wrong path to contacts file " + fileName).c_str());

	while (getline(file, fileLine))
	{
		if (fileLine.empty())
			continue;

		if (fileLine.at(0) == '#')
			continue;

		// This seems to be a valid command line, parse it
		stringstream stringLine(fileLine);
		stringLine >> a >> command >> start >> end >> sourceEid >> destinationEid >> dataRateOrRange >> failureProbability;

		if (failureProb >= 0)
		{
			failureProbability = failureProb;
		}

		if (a.compare("a") == 0)
		{
			if ((command.compare("contact") == 0))
			{
				this->addContact(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0, (double) failureProbability / 100);
			}
			else if ((command.compare("range") == 0))
			{
				this->addRange(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0);
			}
			else if ((command.compare("ocontact") == 0))
			{
				if (mode < 1)
				{
					continue;
				}
				else
				{
				this->addDiscoveredContact(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0, 0);
				this->addDiscoveredContact(start, end, destinationEid, sourceEid, dataRateOrRange, (double) 1.0, 0);
				}
			}
			else if ((command.compare("orange") == 0))
			{
				if (mode < 1)
				{
					continue;
				}
				else
				{
				this->addRange(start, end, sourceEid, destinationEid, dataRateOrRange, (double) 1.0);
				this->addRange(start, end, destinationEid, sourceEid, dataRateOrRange, (double) 1.0);
				}
			}
			else
			{
				cout << "dtnsim error: unknown contact plan command type: a " << fileLine << endl;
			}
		}
	}

	if (cin.bad())
	{
		// IO error
	}
	else if (!cin.eof())
	{
		// format error (not possible with getline but possible with operator>>)
	}
	else
	{
		// format error (not possible with getline but possible with operator>>)
		// or end of file (can't make the difference)
	}

	file.close();

	this->setContactsFile(fileName);
	this->updateContactRanges();
	this->sortContactIdsBySrcByStartTime();
}

int ContactPlan::addContact(double start, double end, int sourceEid, int destinationEid, double dataRate, double confidence)
{
	int id = this->nextContactId++;
	Contact contact(id, start, end, sourceEid, destinationEid, dataRate, confidence, 0);

	contactIdShift_.push_back(id - contacts_.size());
	contacts_.push_back(contact);

	contactIdsBySrc_[sourceEid].push_back(id);

	lastEditTime = simTime();
	this->contactDeleteTimes_[end].push_back(id);
	return id;
}

int ContactPlan::addContact(double start, double end, int sourceEid, int destinationEid, double dataRate, double confidence, double failureProbability)
{
	int id = this->nextContactId++;
	Contact contact(id, start, end, failureProbability, sourceEid, destinationEid, dataRate, confidence, 0);

	contactIdShift_.push_back(id - contacts_.size());
	contacts_.push_back(contact);

	contactIdsBySrc_[sourceEid].push_back(id);

	lastEditTime = simTime();
	this->contactDeleteTimes_[end].push_back(id);
	return id;
}

int ContactPlan::addDiscoveredContact(double start, double end, int sourceEid, int destinationEid, double dataRate, double confidence)
{

	if (this->overlapsWithContact(sourceEid, destinationEid, start, end))
	{
		return -1;

	}

	//this->removePredictedContactsForSourceDestination(sourceEid, destinationEid);
	int id = this->nextContactId++;
	Contact contact(id, start, end, sourceEid, destinationEid, dataRate, confidence, 0, true, false);

	contactIdShift_.push_back(id - contacts_.size());
	contacts_.push_back(contact);

	contactIdsBySrc_[sourceEid].push_back(id);

	lastEditTime = simTime();
	return id;
}

int ContactPlan::addDiscoveredContact(double start, double end, int sourceEid, int destinationEid, double dataRate, double confidence, double failureProbability)
{

	if (this->overlapsWithContact(sourceEid, destinationEid, start, end))
	{
		return -1;

	}

	//this->removePredictedContactsForSourceDestination(sourceEid, destinationEid);
	int id = this->nextContactId++;
	Contact contact(id, start, end, failureProbability, sourceEid, destinationEid, dataRate, confidence, 0, true, false);

	contactIdShift_.push_back(id - contacts_.size());
	contacts_.push_back(contact);

	contactIdsBySrc_[sourceEid].push_back(id);

	lastEditTime = simTime();
	return id;
}


int ContactPlan::addPredictedContact(double start, double end, int sourceEid, int destinationEid, double dataRate, double confidence)
{

	if (this->overlapsWithContact(sourceEid, destinationEid, start, end))
	{
		return -1;
	}
	int id = this->nextContactId++;
	Contact contact(id, start, end, 1 - confidence, sourceEid, destinationEid, dataRate, confidence, 0, false, true);

	contactIdShift_.push_back(id - contacts_.size());
	contacts_.push_back(contact);

	contactIdsBySrc_[sourceEid].push_back(id);

	lastEditTime = simTime();
	return id;
}

void ContactPlan::addRange(double start, double end, int sourceEid, int destinationEid, double range, double confidence)
{
	// Ranges can be declared in a single direction, but they are bidirectional
	// In the worst case they are repeated
	Contact contact1(-1, start, end, sourceEid, destinationEid, 0, confidence, range);
	Contact contact2(-1, start, end, destinationEid, sourceEid, 0, confidence, range);

	ranges_.push_back(contact1);
	ranges_.push_back(contact2);

	lastEditTime = simTime();
}

double ContactPlan::getRangeBySrcDst(int Src, int Dst)
{

	double rangeFirstContact = -1;

	for (size_t i = 0; i < ranges_.size(); i++)
	{
		if ((ranges_.at(i).getSourceEid() == Src) && (ranges_.at(i).getDestinationEid() == Dst))
		{
			rangeFirstContact = ranges_.at(i).getRange();
		}
	}

	return rangeFirstContact;
}

// Each contact matches its position in the vector with the formula:
// position = id - contactIdShift[id]
Contact* ContactPlan::getContactById(int id)
{
	Contact *contactPtr = NULL;

	if (contactIdShift_.at(id) != DELETED_CONTACT)
	{
		contactPtr = &contacts_.at(id - contactIdShift_.at(id));
	}

	return contactPtr;
}

vector<Contact>* ContactPlan::getContacts()
{
	return &contacts_;
}

vector<Contact>* ContactPlan::getRanges()
{
	return &ranges_;
}

vector<int>* ContactPlan::getContactIdsBySrc(int Src)
{
	return &contactIdsBySrc_.at(Src);
}

vector<Contact> ContactPlan::getContactsBySrc(int Src)
{
	vector<Contact> contacts;

	for (size_t i = 0; i < contacts_.size(); i++)
	{
		if (contacts_.at(i).getSourceEid() == Src)
		{
			contacts.push_back(contacts_.at(i));
		}
	}

	return contacts;
}

vector<Contact> ContactPlan::getContactsByDst(int Dst)
{
	vector<Contact> contacts;

	for (size_t i = 0; i < contacts_.size(); i++)
	{
		if (contacts_.at(i).getDestinationEid() == Dst)
		{
			contacts.push_back(contacts_.at(i));
		}
	}

	return contacts;
}

vector<Contact> ContactPlan::getContactsBySrcDst(int Src, int Dst)
{
	vector<Contact> contacts;

	for (size_t i = 0; i < contacts_.size(); i++)
	{
		if ((contacts_.at(i).getSourceEid() == Src) && (contacts_.at(i).getDestinationEid() == Dst))
		{
			contacts.push_back(contacts_.at(i));
		}
	}

	return contacts;
}

/*
 * Returns all discovered contacts
 *
 * @return A vector that contains discovered contacts
 *
 * @author Simon Rink
 */
vector<Contact> ContactPlan::getDiscoveredContacts()
{
	vector<Contact> contacts;

	for (size_t i = 0; i < this->contacts_.size(); i++)
	{
		if (this->contacts_.at(i).isDiscovered())
		{
			contacts.push_back(contacts_.at(i));
		}
	}

	return contacts;
}

/**
 * Returns a pointer to the contact with the given source/destination pair, that starts after the passed start time
 *
 * @param sourceEid: The source EID of the contact
 * 	      destinationEid: The destination EID of the contact
 * 	      start: The start time of the contact
 *
 * @return A pointer to the contact, if it exists, NULL if not
 *
 * @author Simon Rink
 */
Contact* ContactPlan::getContactBySrcDstStart(int sourceEid, int destinationEid, double start)
{
	for (size_t i = 0; i < this->contacts_.size(); i++)
	{
		if (this->contacts_.at(i).getSourceEid() == sourceEid && this->contacts_.at(i).getDestinationEid() == destinationEid && this->contacts_.at(i).getStart() <= start && this->contacts_.at(i).getEnd() > start)
		{
			return &contacts_.at(i);
		}
	}

	return NULL;
}

/*
 * Returns all current neighbors
 *
 * @return A vector that contains all current neighbors
 *
 * @author Simon Rink
 */
vector<int> ContactPlan::getCurrentNeighbors()
{
	return this->currentNeighbors_;
}

simtime_t ContactPlan::getLastEditTime()
{
	return lastEditTime;
}

void ContactPlan::setContactsFile(string contactsFile)
{
	contactsFile_ = contactsFile;
}

const string& ContactPlan::getContactsFile() const
{
	return contactsFile_;
}

void ContactPlan::printContactPlan()
{
	vector<Contact>::iterator it;
	for (it = this->getContacts()->begin(); it != this->getContacts()->end(); ++it)
		cout << "a contact +" << (*it).getStart() << " +" << (*it).getEnd() << " " << (*it).getSourceEid() << " " << (*it).getDestinationEid() << " " << (*it).getResidualVolume() << "/" << (*it).getVolume() << endl;

	for (it = this->getRanges()->begin(); it != this->getRanges()->end(); ++it)
		cout << "a range +" << (*it).getStart() << " +" << (*it).getEnd() << " " << (*it).getSourceEid() << " " << (*it).getDestinationEid() << " " << (*it).getRange() << endl;
	cout << endl;
}

vector<Contact>::iterator ContactPlan::deleteContactById(int contactId)
{
	vector<Contact>::iterator itReturn;

	if (contactIdShift_.at(contactId) == DELETED_CONTACT)
	{
		cout << "Warning: Trying to delete a contact already deleted" << endl;
		return itReturn;
	}

	// Update contactIdsBySrc
	int src = getContactById(contactId)->getSourceEid();
	auto it = std::find(contactIdsBySrc_[src].begin(), contactIdsBySrc_[src].end(), contactId);
	if (it != contactIdsBySrc_[src].end())
		contactIdsBySrc_[src].erase(it);

	// Erase contact
	int contactIndex = contactId - contactIdShift_.at(contactId);
	itReturn = contacts_.erase(contacts_.begin() + contactIndex);

	// Update shift indices
	contactIdShift_.at(contactId) = DELETED_CONTACT;
	for (size_t shiftIndex = contactId + 1; shiftIndex < contactIdShift_.size(); shiftIndex++)
	{
		if (contactIdShift_[shiftIndex] != DELETED_CONTACT)
			contactIdShift_[shiftIndex]++;
	}
	return itReturn;
}

/**
 * Removes a predicted contact for the given source/destination pair
 *
 * @param sourceEid: The EID of the contact source
 *        destinationEid: The EID of the contact destination
 *
 * @return The deleted contact
 *
 * @author Simon Rink
 */
Contact ContactPlan::removePredictedContactForSourceDestination(int sourceEid, int destinationEid)
{
	vector<Contact> contacts = this->getContactsBySrcDst(sourceEid, destinationEid);

	for (size_t i = 0; i < contacts.size(); i++)
	{
		if (contacts.at(i).isPredicted())
		{
			Contact contact = contacts.at(i);
			this->deleteContactById(contacts.at(i).getId());
			lastEditTime = simTime();
			return contact;
		}
	}

	return Contact(-1, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * Removes a discovered contact for the given source/destination pair
 *
 * @param sourceEid: The EID of the contact source
 *        destinationEid: The EID of the contact destination
 *
 * @return The deleted contact
 *
 * @author Simon Rink
 */
Contact ContactPlan::removeDiscoveredContact(int sourceEid, int destinationEid)
{
	vector<Contact> contacts = this->getContactsBySrcDst(sourceEid, destinationEid);

	for (size_t i = 0; i < contacts.size(); i++)
	{
		if (contacts.at(i).isDiscovered())
		{
			Contact contact = contacts.at(i);
			this->deleteContactById(contacts.at(i).getId());
			lastEditTime = simTime();
			return contact;
		}
	}
	return Contact(-1, 0, 0, 0, 0, 0, 0, 0);
}






void ContactPlan::updateContactRanges()
{
	for (auto &rangeContact : ranges_)
	{
		int sourceEid = rangeContact.getSourceEid();
		int destinationEid = rangeContact.getDestinationEid();
		double start = rangeContact.getStart();
		double end = rangeContact.getEnd();

		for (auto &contactId : *getContactIdsBySrc(sourceEid))
		{
			Contact *contact = getContactById(contactId);
			if (contact->getDestinationEid() == destinationEid && contact->getStart() >= start && contact->getEnd() <= end)
			{

				contact->setRange(rangeContact.getRange());
			}
		}
	}
}

void ContactPlan::sortContactIdsBySrcByStartTime()
{
	for (auto &vectorOfIds : this->contactIdsBySrc_)
	{
		std::sort(vectorOfIds.begin(), vectorOfIds.end(), [this](int id1, int id2)
		{
			return this->getContactById(id1)->getStart() <
			this->getContactById(id2)->getStart();
		}
		);
	}
}

/**
 * Checks whether there already exists a contacts that overlaps with the given parameters
 *
 * @param sourceEid: The source EID of the new contact
 * 	      destinationEid: The destination EID of the new contact
 * 	      start: The start time of the contact
 * 	      end: The end time of the contact
 *
 * @return True, if a overlap was found, False if not
 *
 * @author Simon Rink
 */
bool ContactPlan::overlapsWithContact(int sourceEid, int destinationEid, double start, double end)
{
	vector<Contact> contacts = this->getContactsBySrcDst(sourceEid, destinationEid);

	for (size_t i = 0; i < contacts.size(); i++)
	{

		double contactStart = contacts.at(i).getStart();
		double contactEnd = contacts.at(i).getEnd();

		if ((start < contactStart && end > contactStart) || (start >= contactStart && start < contactEnd))
		{
			if (!contacts.at(i).isPredicted())
				return true;
		}

	}

	return false;
}

/**
 * Goes through the list of contacts for the given source/destination pair and returns whether there exists a predicted contact for them
 *
 * @param sourceEid: The EID of the contact source
 * 		  destinationEid: The EID of the contact destination
 *
 * @return Boolean, whether there exists a predicted contact or not
 *
 * @author Simon Rink
 */
bool ContactPlan::hasPredictedContact(int sourceEid, int destinationEid)
{
	vector<Contact> contacts = this->getContactsBySrcDst(sourceEid, destinationEid);

	for (size_t i = 0; i < contacts.size(); i++)
	{
		if (contacts.at(i).isPredicted())
		{
			return true;
		}
	}

	return false;
}

/**
 * Goes through the list of contacts for the given source/destination pair and returns whether there exists a discovered contact for them
 *
 * @param sourceEid: The EID of the contact source
 * 		  destinationEid: The EID of the contact destination
 *
 * @return Boolean, whether there exists a discovered contact or not
 *
 * @author Simon Rink
 */
bool ContactPlan::hasDiscoveredContact(int sourceEid, int destinationEid)
{
	vector<Contact> contacts = this->getContactsBySrcDst(sourceEid, destinationEid);

	for (size_t i = 0; i < contacts.size(); i++)
	{

		if (contacts.at(i).isDiscovered())
		{
			return true;
		}
	}

	return false;
}


/*
 * Returns the highest ID in the plan
 *
 * @return The highest ID in the contact plan
 *
 * @author Simon Rink
 */
int ContactPlan::getHighestId()
{
	return this->nextContactId - 1;
}

/**
 * Adds a new neighbor to the list of current neighbors
 *
 * @param eid: The EID of the neighbor
 *
 * @author Simon Rink
 */
void ContactPlan::addCurrentNeighbor(int eid)
{
	this->currentNeighbors_.push_back(eid);
}

/**
 * Remove a neighbor from the list of current neighbors
 *
 * @param eid: The EID of the neighbor
 *
 * @author Simon Rink
 */
void ContactPlan::removeCurrentNeighbor(int eid)
{
	int position = -1;

	for (size_t i = 0; i < this->currentNeighbors_.size(); i++)
	{
		if (eid == this->currentNeighbors_[i])
		{
			position = i;
			break;
		}
	}

	if (position > -1)
	{
		this->currentNeighbors_.erase(this->currentNeighbors_.begin() + position);
	}
}

/**
 * Traverses the notified end timings of the contacts and deletes them, if the time is <= the current time
 *
 * @author Simon Rink
 */
void ContactPlan::deleteOldContacts()
{
	double currTime = simTime().dbl();

	for (auto it = this->contactDeleteTimes_.begin(); it != this->contactDeleteTimes_.end(); it++)
	{
		if (it->first <= currTime)
		{
			for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
			{
				this->deleteContactById((*jt));
			}

			it->second.clear();
		}

	}
}
